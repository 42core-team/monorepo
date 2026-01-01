#include "Game.h"

#include <json-schema.hpp>
using nlohmann::json_schema::json_validator;

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
		std::vector<std::pair<int, std::string>> preFailures;
		std::vector<std::pair<int, json>> debugDataPackets;

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
						// Validate packet structure
						// does not yet mean the contained actions are valid
						try
						{
							json_validator v;
							v.set_root_schema(Config::load_json_schema("packets/client-packet.schema.json"));
							v.validate(msg);
						}
						catch (const std::exception &e)
						{
							Logger::Log(LogLevel::WARNING, "Invalid client message schema from team " +
																   std::to_string(b->getTeamId()) + ": " + e.what() +
																   " (\"" + msg.dump() + "\")");
							gotMsg[b.get()] = true;
							continue;
						}

						// parse debug data
						if (msg.contains("debug_data") && msg["debug_data"].is_array())
						{
							debugDataPackets.emplace_back(b->getTeamId(), msg);
						}

						// parse actions
						Core *core = Board::instance().getCoreByTeamId(b->getTeamId());
						std::vector<std::string> schemaErrors;
						for (auto &a : Action::parseActions(msg, &schemaErrors))
							actions.emplace_back(std::move(a), core);
						const int tid = core ? core->getTeamId() : b->getTeamId();
						for (const auto &err : schemaErrors)
							preFailures.emplace_back(tid, err);

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
					ReplayEncoder::instance().setPlace(bb->getTeamId(), Board::instance().getCoreCount() - 1);
					it = bridges_.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		tick(tickCount, actions, serverStartTime, preFailures, debugDataPackets);

		tickCount++;
	}

	// determine winner
	for (const Object &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core && obj.getHP() > 0)
		{
			unsigned int tid = static_cast<const Core &>(obj).getTeamId();
			std::string name = ReplayEncoder::instance().getTeamNameFromTeamId(tid);
			if (name.empty()) name = "Team" + std::to_string(tid);
			ReplayEncoder::instance().setDeathReason(tid, death_reason_t::NONE_SURVIVED);
			ReplayEncoder::instance().setPlace(tid, 0);
			Logger::Log("Team " + std::to_string(tid) + " (" + name + ") won the game!");
		}
	}

	Logger::Log("Game ended! Saving replay...");
	ReplayEncoder::instance().exportReplay();
}

void Game::tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions,
				std::chrono::steady_clock::time_point serverStartTime,
				const std::vector<std::pair<int, std::string>> &preFailures,
				const std::vector<std::pair<int, json>> &debugDataPackets)
{
	std::vector<std::pair<int, std::string>> failures;
	failures.reserve(preFailures.size());
	for (const auto &pf : preFailures)
		failures.emplace_back(pf.first, "Tick " + std::to_string(tick - 1) + ": " + pf.second);

	// 0. HANDLE DEBUG INFO
	for (const auto &debugPacket : debugDataPackets)
	{
		int teamId = debugPacket.first;
		for (const auto &debugEntry : debugPacket.second["debug_data"])
		{
			unsigned int objectId = debugEntry["object_id"];
			Object *obj = Board::instance().getObjectById(objectId);
			if (!obj) continue;
			if (obj->getType() == ObjectType::Core)
				if (static_cast<Core *>(obj)->getTeamId() != (unsigned int)teamId) continue;
			if (obj->getType() == ObjectType::Unit)
				if (static_cast<Unit *>(obj)->getTeamId() != (unsigned int)teamId) continue;

			if (debugEntry.contains("object_info") && debugEntry["object_info"].is_string() &&
				!debugEntry["object_info"].get<std::string>().empty())
			{
				obj->setDebugInfo(debugEntry["object_info"].get<std::string>());
			}

			if (debugEntry.contains("object_path") && debugEntry["object_path"].is_array())
			{
				if (obj->getType() != ObjectType::Unit)
				{
					failures.emplace_back(teamId, "Tick " + std::to_string(tick - 1) +
														  ": Debug Error: Only units can have debug paths. Object ID " +
														  std::to_string(objectId) + " is not a unit.");
					continue;
				}

				for (const auto &point : debugEntry["object_path"])
				{
					if (point.contains("x") && point.contains("y") && point["x"].is_number_integer() &&
						point["y"].is_number_integer())
					{
						int x = point["x"];
						int y = point["y"];
						obj->addDebugPathPoint(x, y);
					}
				}
			}
		}
	}

	// 1. EXECUTE ACTIONS

	shuffle_actions_vector(actions);

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

	if (tick >= Config::server().timeoutTicks)
	{
		Logger::Log("Maximum game ticks reached. Killing worst player each tick until one remains.");
		killWorstPlayerOnTimeout();
	}
	if (std::chrono::steady_clock::now() - serverStartTime >= std::chrono::milliseconds(Config::server().timeoutMs))
	{
		Logger::Log("Maximum game time reached. Killing worst player each tick until one remains.");
		killWorstPlayerOnTimeout();
	}

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
	shuffle_vector(removeTeamIds);
	for (int i = 0; i < (int)removeTeamIds.size(); i++)
	{
		unsigned int tid = removeTeamIds[i];

		for (auto it = bridges_.begin(); it != bridges_.end(); ++it)
		{
			if ((*it)->getTeamId() == tid)
			{
				ReplayEncoder::instance().setDeathReason(tid, death_reason_t::CORE_DESTROYED);
				unsigned int place = Board::instance().getCoreCount();
				if (removeTeamIds.size() > 1) place += i; // if multiple died at once, place them randomly
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

	// 8. ActionCooldown / SpawnCooldown DECREMENT FOR UNITS / CORES
	// must happen AFTER state send cause clients & visualizer also do it locally for replay efficiency, otherwise we get a server/client desync with two decrements in one tick when ActionCooldown is reset

	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Unit)
			static_cast<Unit &>(obj).tickActionCooldown();
		else if (obj.getType() == ObjectType::Core)
			static_cast<Core &>(obj).tickSpawnCooldown();
	}

	// 9. Clean up debug info / paths
	for (auto &obj : Board::instance())
	{
		if (obj.hasDebugInfo()) obj.resetDebugInfo();
		if (obj.hasDebugPath()) obj.resetDebugPath();
	}
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
