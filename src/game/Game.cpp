#include "Game.h"

Game::Game(std::vector<unsigned int> team_ids)
	: board_(Config::getInstance().width, Config::getInstance().height), teamCount_(team_ids.size()), nextObjectId_(1)
{
	std::vector<unsigned int> team_ids_double = team_ids;
	shuffle_vector(team_ids_double); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		board_.addObject<Core>(Core(board_.getNextObjectId(), team_ids_double[i], Config::getCorePosition(i)), true);
	Config::getInstance().worldGenerator->generateWorld(this);
	Logger::Log("Game created with " + std::to_string(team_ids.size()) + " teams.");
}
Game::~Game()
{
	for (auto bridge : bridges_)
		delete bridge;
}

void Game::addBridge(Bridge *bridge)
{
	bridges_.push_back(bridge);
}

void Game::run()
{
	sendConfig();

	using clock = std::chrono::steady_clock;
	unsigned int ticksPerSecond = Config::getInstance().tickRate;
	auto tickInterval = std::chrono::nanoseconds(1000000000 / ticksPerSecond);

	auto startTime = clock::now();
	unsigned long long tickCount = 0;

	int alivePlayers = teamCount_;

	while (alivePlayers > 1) // CORE GAMELOOP
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

		alivePlayers = 0;
		for (const auto &obj : board_)
		{
			if (obj.getType() != ObjectType::Core)
				continue;
			if (obj.getHP() > 0)
				alivePlayers++;
		}

		tickCount++;
	}

	Logger::Log("Game ended! Saving replay...");
	json config = Config::encodeConfig();
	replayEncoder_.includeConfig(config);
	replayEncoder_.saveReplay();
}

void Game::tick(unsigned long long tick)
{
	// 1. COMPUTE ACTIONS

	std::vector<std::pair<Action *, Core &>> actions; // action, team id

	for (auto bridge : bridges_)
	{
		json msg;
		bridge->receiveMessage(msg);

		Core *core = board_.getCoreByTeamId(bridge->getTeamId());

		if (!core)
			continue;
		if (core->getHP() <= 0)
			continue;

		std::vector<Action *> clientActions = Action::parseActions(msg);

		for (Action *action : clientActions)
			actions.emplace_back(action, *core);
	}

	shuffle_vector(actions); // shuffle action execution order to ensure fairness

	for (int i = 0; i < (int)actions.size(); i++)
	{
		Action *action = actions[i].first;
		Core &core = actions[i].second;

		if (!action->execute(this, &core))
		{
			delete action;
			actions[i].first = nullptr;
		}
	}

	// 2. UPDATE OBJECTS

	for (auto &obj : board_)
	{
		if (obj.getHP() <= 0)
		{
			if (obj.getType() == ObjectType::Unit && ((Unit &)obj).getBalance() > 0)
			{
				Position objPos = obj.getPosition();
				unsigned int unitBalance = ((Unit &)obj).getBalance();
				board_.removeObjectById(obj.getId());
				board_.addObject<Money>(Money(board_.getNextObjectId(), objPos, unitBalance));
			}
			else if (obj.getType() == ObjectType::Core)
			{
				teamCount_--;
			}
			else
			{
				board_.removeObjectById(obj.getId());
			}
			// TODO: handle game over (send message, disconnect bridge, decrease teamCount_)
		}
		else
		{
			obj.tick(tick, this);
		}
	}

	// 3. SEND STATE

	sendState(actions, tick);
	visualizeGameState(tick);

	for (auto &action : actions)
	{
		if (action.first != nullptr)
			delete action.first;
	}
}

void Game::sendState(std::vector<std::pair<Action *, Core &>> actions, unsigned long long tick)
{
	json state = encodeState(actions, tick);

	replayEncoder_.addTickState(state);

	for (auto bridge : bridges_)
		bridge->sendMessage(state);
}
void Game::sendConfig()
{
	json config = Config::encodeConfig();

	for (auto bridge : bridges_)
	{
		bridge->sendMessage(config);
	}
}
