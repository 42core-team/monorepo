#include "Game.h"

#include <memory>

Game::Game(std::vector<unsigned int> team_ids, GameServiceImpl *service) : service_(service)
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

void Game::run()
{
	auto serverStartTime = std::chrono::steady_clock::now();

	unsigned long long tickCount = 0;
	unsigned int maxWait = Config::server().clientWaitTimeoutMs;

	while (Board::instance().getCoreCount() > 1) // CORE GAMELOOP
	{
		// 1. Signal tick start to all sessions
		auto sessions = service_->getAllSessions();
		for (auto &session : sessions)
		{
			if (session->isDisconnected()) continue;
			unsigned int tid = session->getTeamId();
			std::vector<std::string> errors;
			auto it = pendingErrors_.find(tid);
			if (it != pendingErrors_.end())
			{
				errors = std::move(it->second);
				pendingErrors_.erase(it);
			}
			session->signalTickStart(tickCount, errors, false);
		}
		service_->setCurrentTick(tickCount);

		// 2. Wait for all EndTurn messages (or timeout)
		for (auto &session : sessions)
		{
			if (session->isDisconnected()) continue;
			if (!session->waitForEndTurn(maxWait))
			{
				unsigned int tid = session->getTeamId();
				Logger::LogWarn("Session of team " + std::to_string(tid) +
								" did not send EndTurn in time. Disconnecting.");
				if (session->isDisconnected())
					ReplayEncoder::instance().setDeathReason(tid, death_reason_t::DISCONNECTED);
				else
					ReplayEncoder::instance().setDeathReason(tid, death_reason_t::TIMEOUT_SENDING_DATA);
				ReplayEncoder::instance().setPlace(tid, Board::instance().getCoreCount() - 1);
				session->setDisconnected();

				// Remove core from board (like Bridge destructor did)
				auto core = Board::instance().getCoreByTeamId(tid);
				if (core != nullptr)
				{
					Board::instance().removeObjectById(core->getId());
					Logger::Log("Core of team " + std::to_string(tid) + " has been removed from the board.");
				}
			}
		}

		// 3. Drain actions + debug data from all PlayerSessions
		std::vector<std::pair<std::unique_ptr<Action>, Core *>> actions;
		std::vector<std::pair<int, std::string>> preFailures;
		std::vector<std::pair<int, core_game::DebugDataEntry>> debugDataEntries;

		for (auto &session : sessions)
		{
			if (session->isDisconnected()) continue;
			unsigned int tid = session->getTeamId();
			Core *core = Board::instance().getCoreByTeamId(tid);

			// Drain actions
			auto sessionActions = session->drainActions();
			for (auto &action : sessionActions)
			{
				actions.emplace_back(std::move(action), core);
			}

			// Drain debug data
			auto debugEntries = session->drainDebugData();
			for (auto &entry : debugEntries)
			{
				debugDataEntries.emplace_back(static_cast<int>(tid), std::move(entry));
			}
		}

		// 4. Execute tick
		tick(tickCount, actions, serverStartTime, preFailures, debugDataEntries);

		tickCount++;
	}

	// Determine winner
	int winnerTeamId = -1;
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
			winnerTeamId = static_cast<int>(tid);
		}
	}

	// Send final game-over signal to all sessions
	auto sessions = service_->getAllSessions();
	for (auto &session : sessions)
	{
		if (session->isDisconnected()) continue;
		unsigned int tid = session->getTeamId();
		std::vector<std::string> errors;
		auto it = pendingErrors_.find(tid);
		if (it != pendingErrors_.end())
		{
			errors = std::move(it->second);
			pendingErrors_.erase(it);
		}
		session->signalGameOver(tickCount, errors, winnerTeamId);
	}

	Logger::Log("Game ended! Saving replay...");
	ReplayEncoder::instance().exportReplay();
}

void Game::tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions,
				std::chrono::steady_clock::time_point serverStartTime,
				const std::vector<std::pair<int, std::string>> &preFailures,
				const std::vector<std::pair<int, core_game::DebugDataEntry>> &debugDataEntries)
{
	std::vector<std::pair<int, std::string>> failures;
	failures.reserve(preFailures.size());
	for (const auto &pf : preFailures)
		failures.emplace_back(pf.first, "Tick " + std::to_string(tick - 1) + ": " + pf.second);

	// 0. HANDLE DEBUG INFO
	for (const auto &debugPair : debugDataEntries)
	{
		int teamId = debugPair.first;
		const core_game::DebugDataEntry &debugEntry = debugPair.second;

		unsigned int objectId = debugEntry.object_id();
		Object *obj = Board::instance().getObjectById(objectId);
		if (!obj) continue;
		if (obj->getType() == ObjectType::Core)
			if (static_cast<Core *>(obj)->getTeamId() != (unsigned int)teamId) continue;
		if (obj->getType() == ObjectType::Unit)
			if (static_cast<Unit *>(obj)->getTeamId() != (unsigned int)teamId) continue;

		if (debugEntry.has_object_info() && !debugEntry.object_info().empty())
		{
			obj->setDebugInfo(debugEntry.object_info());
		}

		if (debugEntry.object_path_size() > 0)
		{
			if (obj->getType() != ObjectType::Unit)
			{
				failures.emplace_back(teamId, "Tick " + std::to_string(tick - 1) +
													  ": Debug Error: Only units can have debug paths. Object ID " +
													  std::to_string(objectId) + " is not a unit.");
				continue;
			}

			for (const auto &point : debugEntry.object_path())
			{
				obj->addDebugPathPoint(point.x(), point.y());
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
			json actionJson = action->encodeJSON();

			std::string fullErr = "Tick " + std::to_string(tick - 1) +
								  ": Action Failure: " + Action::getActionName(action->getActionType()) + ": " + err +
								  " (" + actionJson.dump() + ")";

			unsigned int actingUnitInError = 0;
			if (actionJson.contains("unit_id"))
				actingUnitInError = actionJson["unit_id"];
			else if (actionJson.contains("builder_id"))
				actingUnitInError = actionJson["builder_id"];
			if (actingUnitInError != 0)
			{
				Object *obj = Board::instance().getObjectById(actingUnitInError);
				if (obj != NULL)
				{
					if (obj->getDebugInfo().find("[begin_errs]") != std::string::npos)
						obj->setDebugInfo(obj->getDebugInfo() + fullErr + "\n");
					else
						obj->setDebugInfo(obj->getDebugInfo() + "\n[begin_errs]\n" + fullErr + "\n");
				}
			}

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

	// 5. RECORD REPLAY STATE (no longer sends state to clients)

	recordReplayState(actions, tick, failures);
	Visualizer::visualizeGameState(tick);

	// ----------------------------

	// 6. REMOVE CORES
	// Replay needs to see the final state with core at 0 hp

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

		ReplayEncoder::instance().setDeathReason(tid, death_reason_t::CORE_DESTROYED);
		unsigned int place = Board::instance().getCoreCount();
		if (removeTeamIds.size() > 1) place += i;
		ReplayEncoder::instance().setPlace(tid, place);
		if (place == 0)
		{
			ReplayEncoder::instance().setDeathReason(tid, death_reason_t::NONE_SURVIVED);
		}

		// Disconnect the session and remove core
		auto session = service_->getSession(tid);
		if (session)
		{
			session->setDisconnected();
		}

		auto core = Board::instance().getCoreByTeamId(tid);
		if (core != nullptr)
		{
			Board::instance().removeObjectById(core->getId());
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

	// 8. ActionCooldown / SpawnCooldown DECREMENT

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

	// 10. Store failures as pending errors for next tick's TickSignal
	for (const auto &failure : failures)
	{
		pendingErrors_[failure.first].push_back(failure.second);
	}
}

void Game::killWorstPlayerOnTimeout()
{
	if (Board::instance().getCoreCount() <= 1) return;

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

void Game::recordReplayState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick,
							 std::vector<std::pair<int, std::string>> & /*failures*/)
{
	json state = stateEncoder_.generateObjectDiff();

	ReplayEncoder::instance().addTickState(state, tick, actions);
}
