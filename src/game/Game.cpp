#include "Game.h"
#include <memory>

Game::Game(std::vector<unsigned int> team_ids)
	: nextObjectId_(1)
{
	std::vector<unsigned int> team_ids_double = team_ids;
	shuffle_vector(team_ids_double); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		Board::instance().addObject<Core>(Core(Board::instance().getNextObjectId(), team_ids_double[i]), Config::getCorePosition(i), true);
	Config::instance().worldGenerator->generateWorld();
	Logger::Log("Game created with " + std::to_string(team_ids.size()) + " teams.");
}
Game::~Game() {}

void Game::addBridge(std::unique_ptr<Bridge> bridge)
{
	bridges_.emplace_back(std::move(bridge));
}

void Game::run()
{
	sendConfig();

	using clock = std::chrono::steady_clock;
	unsigned int ticksPerSecond = Config::instance().tickRate;
	auto tickInterval = std::chrono::nanoseconds(1000000000 / ticksPerSecond);

	auto startTime = clock::now();
	unsigned long long tickCount = 0;

	while (Board::instance().getCoreCount() > 1) // CORE GAMELOOP
	{
		auto now = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime);
		unsigned long long expectedTickCount = elapsed.count() / tickInterval.count();

		if (expectedTickCount > tickCount)
		{
			// Over one tick passed, we're lagging.
			if (expectedTickCount > tickCount + 1)
			{
				unsigned long skipped = expectedTickCount - tickCount - 1;
				Logger::Log(LogLevel::WARNING, "Processing delay: skipping " + std::to_string(skipped) + " tick(s).");
			}
			tickCount = expectedTickCount;
		}

		auto nextTickTime = startTime + (tickCount + 1) * tickInterval;
		std::this_thread::sleep_until(nextTickTime);

		tick(tickCount);

		tickCount++;
	}

	// determine winner
	for (const auto& bridge : bridges_)
	{
		if (!bridge->isDisconnected())
		{
			replayEncoder_.addTeamScore(bridge->getTeamId(), bridge->getTeamName(), 0); // 0 = won
			Logger::Log("Team " + std::to_string(bridge->getTeamId()) + " (" + bridge->getTeamName() + ") won the game!");
		}
	}

	Logger::Log("Game ended! Saving replay...");
	json config = Config::encodeConfig();
	replayEncoder_.includeConfig(config);
	replayEncoder_.saveReplay();
}

void Game::tick(unsigned long long tick)
{
	// print brdiges and ids for debug
	for (const auto& bridge : bridges_)
	{
		Logger::Log("Bridge for team " + std::to_string(bridge->getTeamId()) + " is connected.");
	}
	Logger::Log("Tick " + std::to_string(tick) + " started.");


	// 1. COMPUTE ACTIONS

	std::vector<std::pair<std::unique_ptr<Action>, Core &>> actions; // action, team id

	for (auto& bridge : bridges_)
	{
		json msg;
		bridge->receiveMessage(msg);

		Core *core = Board::instance().getCoreByTeamId(bridge->getTeamId());

		if (!core)
			continue;
		if (core->getHP() <= 0)
			continue;

		std::vector<std::unique_ptr<Action>> clientActions = Action::parseActions(msg);

		for (auto& action : clientActions)
			actions.emplace_back(std::move(action), *core);
	}

	shuffle_vector(actions); // shuffle action execution order to ensure fairness

	for (auto& ele : actions) {
		auto& action = ele.first;
		Core& core = ele.second;

		if (!action->execute(&core))
			action = nullptr;
	}

	// 2. UPDATE OBJECTS

	for (auto &obj : Board::instance())
	{
		if (obj.getHP() <= 0)
		{
			if (obj.getType() == ObjectType::Unit && ((Unit &)obj).getBalance() > 0)
			{
				Position objPos = Board::instance().getObjectPositionById(obj.getId());
				unsigned int unitBalance = ((Unit &)obj).getBalance();
				Board::instance().removeObjectById(obj.getId());
				Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId(), unitBalance), objPos);
			}
			else if (obj.getType() != ObjectType::Core)
			{
				Board::instance().removeObjectById(obj.getId());
			}
			// TODO: handle game over (send message, disconnect bridge, decrease teamCount_)
		}
		else
		{
			obj.tick(tick);
		}
	}

	// 3. SEND STATE

	sendState(actions, tick);
	Visualizer::visualizeGameState(tick);

	// 4. REMOVE CORES
	// connection libs must receive one final state json with their core at 0 hp to realize they lost
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core && obj.getHP() <= 0)
		{
			Core &core = (Core &)obj;
			for (auto &bridge : bridges_)
			{
				if (!bridge->isDisconnected() && bridge->getTeamId() == core.getTeamId())
				{
					replayEncoder_.addTeamScore(bridge->getTeamId(), bridge->getTeamName(), Board::instance().getCoreCount() - 1);
					bridges_.erase(std::remove(bridges_.begin(), bridges_.end(), bridge), bridges_.end());
					break;
				}
			}
			
			// remove core from board
			Logger::Log("Core of team " + std::to_string(core.getTeamId()) + " has been destroyed.");
			Board::instance().removeObjectById(obj.getId());
		}
	}
}

void Game::sendState(std::vector<std::pair<std::unique_ptr<Action>, Core &>> &actions, unsigned long long tick)
{
	json state = encodeState(actions, tick);

	replayEncoder_.addTickState(state);

	for (auto& bridge : bridges_)
		bridge->sendMessage(state);
}
void Game::sendConfig()
{
	json config = Config::encodeConfig();

	for (auto& bridge : bridges_)
	{
		bridge->sendMessage(config);
	}
}
