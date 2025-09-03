#include "Game.h"

#include <memory>

Game::Game(std::vector<unsigned int> team_ids)
{
	shuffle_vector(team_ids); // randomly assign core positions to ensure fairness
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		Board::instance().addObject<Core>(Core(team_ids[i]), Config::getCorePosition(i), true);

	if (Config::game().usedRandomSeed) Logger::Log("Generating world with random seed as no seed was provided.");
	Logger::Log("Generating world with seed \"" + Config::game().seedString +
				"\". (hash: " + std::to_string(Config::game().seed) + ")");
	Config::game().worldGenerator->generateWorld(Config::game().seed);
	ReplayEncoder::instance().getCustomData()["worldGeneratorSeed"] = Config::game().seedString;

	Logger::Log("Game created with " + std::to_string(team_ids.size()) + " teams.");
}
Game::~Game()
{
}

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
		std::unordered_map<Bridge *, bool> gotMsg;
		for (auto &b : bridges_)
			gotMsg[b.get()] = false;

		std::vector<std::pair<std::unique_ptr<Action>, Core *>> actions;
		while (std::chrono::steady_clock::now() - waitStart < std::chrono::milliseconds(maxWait))
		{
			bool all = true;
			for (auto &b : bridges_)
			{
				if (!gotMsg[b.get()])
				{
					json msg;
					if (b->tryReceiveMessage(msg))
					{
						Core *core = Board::instance().getCoreByTeamId(b->getTeamId());
						for (auto &a : Action::parseActions(msg))
							actions.emplace_back(std::move(a), core);
						gotMsg[b.get()] = true;
					}
					else
					{
						all = false;
					}
				}
			}
			if (all) break;
		}
		if (std::chrono::steady_clock::now() - waitStart >= std::chrono::milliseconds(maxWait))
		{
			for (auto it = bridges_.begin(); it != bridges_.end();)
			{
				Bridge *bb = it->get();
				if (!gotMsg[bb])
				{
					Logger::LogWarn("Bridge of team " + std::to_string(bb->getTeamId()) +
									" did not send an action in time. Disconnecting.");
					for (auto &action : actions)
					{
						if (action.second && action.second->getTeamId() == bb->getTeamId())
						{
							action.second = nullptr; // invalidate actions for this team
						}
					}
					if (bb->isDisconnected())
						ReplayEncoder::instance().setDeathReason(bb->getTeamId(), death_reason_t::DISCONNECTED);
					else
						ReplayEncoder::instance().setDeathReason(bb->getTeamId(), death_reason_t::TIMEOUT_SENDING_DATA);
					ReplayEncoder::instance().setPlace(bb->getTeamId(), Board::instance().getCoreCount());
					it = bridges_.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		tick(tickCount, actions, serverStartTime);

		tickCount++;
	}

	// determine winner
	for (const Object &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core && obj.getHP() > 0)
		{
			unsigned int tid = static_cast<const Core &>(obj).getTeamId();
			std::string name = "Team" + std::to_string(tid);
			for (const auto &b : bridges_)
				if (b->getTeamId() == tid)
				{
					name = b->getTeamName();
					break;
				}
			ReplayEncoder::instance().setDeathReason(tid, death_reason_t::NONE_SURVIVED);
			ReplayEncoder::instance().setPlace(tid, 0);
			Logger::Log("Team " + std::to_string(tid) + " (" + name + ") won the game!");
		}
	}

	Logger::Log("Game ended! Saving replay...");
	ReplayEncoder::instance().exportReplay();
}

void Game::tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions,
				std::chrono::steady_clock::time_point serverStartTime)
{
	std::vector<std::pair<int, std::string>> failures; // errors for clients

	// 1. EXECUTE ACTIONS

	shuffle_vector(actions); // shuffle action execution order to ensure fairness

	for (auto &ele : actions)
	{
		auto &action = ele.first;
		Core *core = ele.second;
		if (!core || !action)
		{
			action = nullptr;
			continue;
		}

		std::string err = action->execute(core);
		if (!err.empty())
		{
			// makes more sense to put the ticks where the actions were executed into the tick message
			std::string fullErr = "Tick " + std::to_string(tick - 1) +
								  ": Action Failure: " + Action::getActionName(action->getActionType()) + ": " + err +
								  " (" + action->encodeJSON().dump() + ")";
			failures.emplace_back(core->getTeamId(), fullErr);
			action = nullptr;
		}
	}

	// 2. TICK OBJECTS

	for (auto &obj : Board::instance())
		obj.tick(tick);

	// 3. DELETE DEAD OBJECTS

	for (auto &obj : Board::instance())
	{
		if (obj.getHP() > 0) continue;

		switch (obj.getType())
		{
		case ObjectType::Unit:
			Stats::instance().inc(stat_keys::units_destroyed);
			break;
		case ObjectType::Wall:
			Stats::instance().inc(stat_keys::walls_destroyed);
			break;
		case ObjectType::Core:
			Stats::instance().inc(stat_keys::cores_destroyed);
			break;
		case ObjectType::Bomb:
			Stats::instance().inc(stat_keys::bombs_destroyed);
			break;
		default:
			break;
		}

		if (obj.getType() == ObjectType::Unit)
		{
			Position objPos = Board::instance().getObjectPositionById(obj.getId());
			unsigned int unitBalance = ((Unit &)obj).getBalance();

			Board::instance().removeObjectById(obj.getId());

			if (unitBalance > 0) Board::instance().addObject<GemPile>(GemPile(unitBalance), objPos);
		}
		// Cores must stay so clients know they died, Bombs must stay so the visualizer can get encoded positions where the explosion happened
		else if (obj.getType() != ObjectType::Core && obj.getType() != ObjectType::Bomb)
		{
			Board::instance().removeObjectById(obj.getId());
		}
	}

	// 4. CHECK TIMEOUT

	if (tick >= Config::server().timeoutTicks) killWorstPlayerOnTimeout();
	if (std::chrono::steady_clock::now() - serverStartTime >= std::chrono::milliseconds(Config::server().timeoutMs))
		killWorstPlayerOnTimeout();

	// 5. SEND STATE

	sendState(actions, tick, failures);
	Visualizer::visualizeGameState(tick);


	// ----------------------------


	// 6. REMOVE CORES
	// connection libs must receive one final state json with their core at 0 hp to realize they lost

	std::vector<unsigned> removeTeamIds;
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core && obj.getHP() <= 0)
			removeTeamIds.push_back(static_cast<Core &>(obj).getTeamId());
	}
	for (unsigned tid : removeTeamIds)
	{
		for (auto it = bridges_.begin(); it != bridges_.end(); ++it)
		{
			if ((*it)->getTeamId() == tid)
			{
				ReplayEncoder::instance().setDeathReason(tid, death_reason_t::CORE_DESTROYED);
				unsigned int place = Board::instance().getCoreCount();
				ReplayEncoder::instance().setPlace(tid, place);
				if (place == 0)
				{
					ReplayEncoder::instance().setDeathReason(tid, death_reason_t::NONE_SURVIVED);
				}
				bridges_.erase(it);
				break;
			}
		}
	}

	// 7. REMOVE BOMBS

	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Bomb && obj.getHP() <= 0)
		{
			Board::instance().removeObjectById(obj.getId());
		}
	}

	// 8. ActionCooldown DECREMENT FOR UNITS
	// must happen AFTER state send cause clients also do it locally for replay efficiency, otherwise we get a server/client desync with two decrements in one tick when ActionCooldown is reset

	for (auto &obj : Board::instance())
		if (obj.getType() == ObjectType::Unit) static_cast<Unit &>(obj).tickActionCooldown();
}

void Game::killWorstPlayerOnTimeout()
{
	if (Board::instance().getCoreCount() <= 1) return;

	// determine winner: go to next number if still no clear winner

	// 1. Least core hp
	Core *weakest = nullptr;
	bool tie = false;
	for (auto &obj : Board::instance())
	{
		if (obj.getType() != ObjectType::Core) continue;
		Core &core = (Core &)obj;
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
		Logger::Log("Killing core of team " + std::to_string(weakest->getTeamId()) +
					" due to timeout (least core hp).");
		weakest->setHP(0);
		ReplayEncoder::instance().setDeathReason(weakest->getTeamId(), death_reason_t::TIMEOUT_CORE_HP);
		return;
	}

	// 2. Least unit hp total
	unsigned int minUnitHp = std::numeric_limits<unsigned int>::max();
	Core *minUnitHpCore = nullptr;
	tie = false;
	for (auto &obj : Board::instance())
	{
		if (obj.getType() != ObjectType::Core) continue;
		Core &core = (Core &)obj;
		unsigned int teamId = core.getTeamId();
		unsigned int totalUnitHp = 0;

		for (auto &unit : Board::instance())
			if (unit.getType() == ObjectType::Unit && ((Unit &)unit).getTeamId() == teamId) totalUnitHp += unit.getHP();

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
		Logger::Log("Killing core of team " + std::to_string(minUnitHpCore->getTeamId()) +
					" due to timeout (least unit hp total).");
		ReplayEncoder::instance().setDeathReason(minUnitHpCore->getTeamId(), death_reason_t::TIMEOUT_UNIT_HP);
		minUnitHpCore->setHP(0);
		return;
	}

	// 3. Random pick
	unsigned int remainingCores = Board::instance().getCoreCount();
	std::uniform_int_distribution<unsigned> dist(0, remainingCores - 1);
	unsigned int randomIndex = dist(rng_);
	Object *randomCore = nullptr;
	for (auto &obj : Board::instance())
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
		unsigned int teamId = ((Core *)randomCore)->getTeamId();
		Logger::Log("Killing core of team " + std::to_string(teamId) + " due to timeout (random pick).");
		ReplayEncoder::instance().setDeathReason(teamId, death_reason_t::TIMEOUT_RANDOM);
		randomCore->setHP(0);
		return;
	}
	else
	{
		Logger::Log(LogLevel::ERROR, "No core found for random pick on timeout. This should not happen.");
		return;
	}
}

void Game::sendState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick,
					 std::vector<std::pair<int, std::string>> &failures)
{
	json state = stateEncoder_.generateObjectDiff();

	ReplayEncoder::instance().addTickState(state, tick, actions);

	state["tick"] = tick;

	for (auto &bridge : bridges_)
	{
		json teamState = state;
		teamState["errors"] = json::array();
		const int teamId = bridge->getTeamId();
		for (const auto &failure : failures)
			if (failure.first == teamId) teamState["errors"].push_back(failure.second);
		bridge->sendMessage(teamState);
	}
}
void Game::sendConfig()
{
	json config = Config::encodeConfig();

	for (auto &bridge : bridges_)
	{
		bridge->sendMessage(config);
	}
}
