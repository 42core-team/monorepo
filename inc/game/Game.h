#ifndef GAME_H
#define GAME_H

#include <vector>

#include "Config.h"
#include "Core.h"
#include "Unit.h"
#include "Bridge.h"

#include "json.hpp"
using json = nlohmann::json;

class Game
{
	public:
		Game(unsigned int teamCount);
		void addBridge(Bridge* bridge);
		void run();

	private:
		void update();
		void sendState();

		unsigned int teamCount_;
		unsigned int nextObjectId_;
		std::vector<Core> cores_;
		std::vector<Unit> units_;
		std::vector<Bridge*> bridges_;
};

#endif // GAME_H
