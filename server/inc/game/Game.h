#ifndef GAME_H
#define GAME_H

#include "Action.h"
#include "Board.h"
#include "Core.h"
#include "Deposit.h"
#include "GameServiceImpl.h"
#include "PlayerSession.h"
#include "ReplayEncoder.h"
#include "StateEncoder.h"
#include "Unit.h"
#include "Utils.h"
#include "Visualizer.h"
#include "Wall.h"
#include "json.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
using json = nlohmann::ordered_json;

class Config;

class Game
{
  public:
	Game(std::vector<unsigned int> team_ids, GameServiceImpl *service);
	~Game();

	void run();

  private:
	void tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions,
			  std::chrono::steady_clock::time_point serverStartTime,
			  const std::vector<std::pair<int, std::string>> &preFailures,
			  const std::vector<std::pair<int, core_game::DebugDataEntry>> &debugDataEntries);

	void killWorstPlayerOnTimeout();

	void recordReplayState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick,
						   std::vector<std::pair<int, std::string>> &failures);

	GameServiceImpl *service_;

	// Pending errors per team, accumulated and sent in next tick's TickSignal
	std::unordered_map<unsigned int, std::vector<std::string>> pendingErrors_;

	std::mt19937 rng_;

	StateEncoder stateEncoder_;
};

#include "Config.h"
#include "WorldGenerator.h"

#endif // GAME_H
