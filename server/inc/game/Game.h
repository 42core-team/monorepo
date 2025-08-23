#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>

#include "Core.h"
#include "Unit.h"
#include "Deposit.h"
#include "Wall.h"
#include "Bridge.h"
#include "Action.h"
#include "Utils.h"
#include "ReplayEncoder.h"
#include "StateEncoder.h"
#include "Board.h"
#include "Visualizer.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class Config;

class Game
{
	public:
		Game(std::vector<unsigned int> team_ids);
		~Game();
		void addBridge(std::unique_ptr<Bridge> bridge);

		void run();

	private:
		void tick(unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, std::chrono::steady_clock::time_point serverStartTime);

		void killWorstPlayerOnTimeout();

		void sendState(std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions, unsigned long long tick, std::vector<std::pair<int,std::string>> &failures);
		void sendConfig();

		std::vector<std::unique_ptr<Bridge>> bridges_;

		std::mt19937 rng_;

		StateEncoder stateEncoder_;
};

#include "Config.h"
#include "WorldGenerator.h"

#endif // GAME_H
