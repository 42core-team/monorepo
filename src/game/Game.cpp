#include "Game.h"
#include <memory>

Game::Game(std::vector<unsigned int> team_ids)
	: nextObjectId_(1)
{
	shuffle_vector(team_ids); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		Board::instance().addObject<Core>(Core(Board::instance().getNextObjectId(), team_ids[i]), Config::getCorePosition(i), true);
	Config::game().worldGenerator->generateWorld();
	Logger::Log("Game created with " + std::to_string(team_ids.size()) + " teams.");
}
Game::~Game() {}

void Game::addBridge(std::unique_ptr<Bridge> bridge)
{
	bridges_.emplace_back(std::move(bridge));
}

void Game::run()
{
	auto serverStartTime = std::chrono::steady_clock::now();

	sendConfig();
	unsigned long long tickCount = 0;

	unsigned int maxWait = Config::server().clientWaitTimeoutMs;
	while (Board::instance().getCoreCount() > 1) // CORE GAMELOOP
	{
		auto waitStart = std::chrono::steady_clock::now();
		std::unordered_map<Bridge*, bool> gotMsg;
		for (auto& b : bridges_) gotMsg[b.get()] = false;

		std::vector<std::pair<std::unique_ptr<Action>, Core *>> actions;
		while (std::chrono::steady_clock::now() - waitStart < std::chrono::milliseconds(maxWait)) {
			bool all = true;
			for (auto& b : bridges_) {
				if (!gotMsg[b.get()]) {
					json msg;
					if (b->tryReceiveMessage(msg)) {
						Core* core = Board::instance().getCoreByTeamId(b->getTeamId());
						for (auto& a : Action::parseActions(msg))
							actions.emplace_back(std::move(a), core);
						gotMsg[b.get()] = true;
					} else {
						all = false;
					}
				}
			}
			if (all) break;
		}
		if (std::chrono::steady_clock::now() - waitStart >= std::chrono::milliseconds(maxWait)) {
			for (auto it = bridges_.begin(); it != bridges_.end();) {
				Bridge* bb = it->get();
				if (!gotMsg[bb]) {
					Logger::LogWarn("Bridge of team " + std::to_string(bb->getTeamId()) + " did not send an action in time. Disconnecting.");
					for (auto& action : actions) {
						if (action.second && action.second->getTeamId() == bb->getTeamId()) {
							action.second = nullptr; // invalidate actions for this team
						}
					}
					it = bridges_.erase(it);
				} else {
					++it;
				}
			}
		}

		tick(tickCount, actions, serverStartTime);

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
	replayEncoder_.exportReplay();
}

void Game::tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, std::chrono::steady_clock::time_point serverStartTime)
{
	// 1. EXECUTE ACTIONS

	shuffle_vector(actions); // shuffle action execution order to ensure fairness

	for (auto& ele : actions) {
		auto& action = ele.first;
		Core *core = ele.second;

		if (!core || !action)
			action = nullptr;
		else if (!action->execute(core))
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

	// 3. CHECK TIMEOUT

	if (tick >= Config::server().timeoutTicks)
		killWorstPlayerOnTimeout();
	if (std::chrono::steady_clock::now() - serverStartTime >= std::chrono::milliseconds(Config::server().timeoutMs))
		killWorstPlayerOnTimeout();

	// 4. SEND STATE

	sendState(actions, tick);
	Visualizer::visualizeGameState(tick);

	// 5. REMOVE CORES
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
					replayEncoder_.addTeamScore(bridge->getTeamId(), bridge->getTeamName(), Board::instance().getCoreCount());
					bridges_.erase(std::remove(bridges_.begin(), bridges_.end(), bridge), bridges_.end());
					break;
				}
			}
		}
	}
}

void Game::killWorstPlayerOnTimeout()
{
	if (Board::instance().getCoreCount() <= 1)
		return;

	// determine winner: go to next number if still no clear winner

	// 1. Least core hp
	Core *weakest = nullptr;
	bool tie = false;
	for (auto& obj : Board::instance())
	{
		if (obj.getType() != ObjectType::Core)
			continue;
		Core& core = (Core&)obj;
		if (!weakest || core.getHP() < weakest->getHP())
		{
			weakest = &core;
			tie = false;
		}
		else if (core.getHP() == weakest->getHP())
		{
			tie = true;
		}
	}
	if (weakest && !tie)
	{
		Logger::Log("Killing core of team " + std::to_string(weakest->getTeamId()) + " due to timeout (least core hp).");
		weakest->setHP(0);
		replayEncoder_.setGameEndReason(GER_CORE_HP);
		return ;
	}

	// 2. Least unit hp total
	unsigned int minUnitHp = std::numeric_limits<unsigned int>::max();
	Core *minUnitHpCore = nullptr;
	tie = false;
	for (auto& obj : Board::instance())
	{
		if (obj.getType() != ObjectType::Core)
			continue;
		Core& core = (Core&)obj;
		unsigned int teamId = core.getTeamId();
		unsigned int totalUnitHp = 0;

		for (auto& unit : Board::instance())
			if (unit.getType() == ObjectType::Unit && ((Unit&)unit).getTeamId() == teamId)
				totalUnitHp += unit.getHP();

		if (totalUnitHp < minUnitHp)
		{
			minUnitHp = totalUnitHp;
			minUnitHpCore = &core;
			tie = false;
		}
		else if (totalUnitHp == minUnitHp && minUnitHpCore)
		{
			tie = true;
		}
	}
	if (minUnitHpCore && !tie)
	{
		Logger::Log("Killing core of team " + std::to_string(minUnitHpCore->getTeamId()) + " due to timeout (least unit hp total).");
		replayEncoder_.setGameEndReason(GER_UNIT_HP);
		minUnitHpCore->setHP(0);
		return ;
	}

	// 3. Random pick
	unsigned int remainingCores = Board::instance().getCoreCount();
	std::uniform_int_distribution<unsigned> dist(0, remainingCores - 1);
	unsigned int randomIndex = dist(rng_);
	Object * randomCore = nullptr;
	for (auto& obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core && obj.getHP() > 0)
		{
			if (randomIndex == 0)
			{
				randomCore = &obj;
				break;
			}
			else
				randomIndex--;
		}
	}
	if (randomCore)
	{
		unsigned int teamId = ((Core*)randomCore)->getTeamId();
		Logger::Log("Killing core of team " + std::to_string(teamId) + " due to timeout (random pick).");
		replayEncoder_.setGameEndReason(GER_RANDOM);
		randomCore->setHP(0);
		return ;
	}
	else
	{
		Logger::Log(LogLevel::ERROR, "No core found for random pick on timeout. This should not happen.");
		return;
	}
}

void Game::sendState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick)
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
