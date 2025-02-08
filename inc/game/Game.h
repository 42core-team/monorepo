#ifndef GAME_H
#define GAME_H

#include <vector>
#include <chrono>
#include <iostream>

#include "Config.h"
#include "Core.h"
#include "Unit.h"
#include "Resource.h"
#include "Wall.h"
#include "Bridge.h"
#include "Action.h"
#include "Utils.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class Game
{
	public:
		Game(std::vector<unsigned int> team_ids);
		~Game();
		void addBridge(Bridge* bridge);

		void run();

		std::vector<Unit> & getUnits() { return units_; }

		Core * getCore(unsigned int teamId);
		Unit * getUnit(unsigned int unitId);

		Object * getObjectAtPos(Position pos);

	private:
		void tick(unsigned long long tick);
		void sendState();
		void sendConfig();

		unsigned int teamCount_;
		unsigned int nextObjectId_;
		std::vector<Core> cores_;
		std::vector<Unit> units_;
		std::vector<Resource> resources_;
		std::vector<Wall> walls_;
		std::vector<Bridge*> bridges_;

		void visualizeGameState();
};

#endif // GAME_H
