#ifndef GAME_H
#define GAME_H

#include <vector>
#include <chrono>
#include <iostream>
#include <fstream>

#include "Core.h"
#include "Unit.h"
#include "Resource.h"
#include "Wall.h"
#include "Bridge.h"
#include "Action.h"
#include "Utils.h"
#include "ReplayEncoder.h"
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
		void addBridge(Bridge* bridge);

		void run();

	private:
		void tick(unsigned long long tick);

		json encodeState(std::vector<std::pair<Action *, Core &>> actions, unsigned long long tick);
		void sendState(std::vector<std::pair<Action *, Core &>> actions, unsigned long long tick);
		void sendConfig();

		unsigned int teamCount_;
		unsigned int nextObjectId_;
		std::vector<Bridge*> bridges_;

		ReplayEncoder replayEncoder_;
};

#include "Config.h"
#include "WorldGenerator.h"

#endif // GAME_H
