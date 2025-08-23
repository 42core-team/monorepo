#pragma once

#include "Config.h"
#include "MapTemplate.h"
#include "WorldGenerator.h"
#include "Logger.h"
#include "Object.h"
#include "Wall.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Board.h"
#include "Visualizer.h"

#include <vector>
#include <string>
#include <filesystem>
#include <queue>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <chrono>
#include <random>

struct Rectangle {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};

class JigsawWorldGenerator : public WorldGenerator {
	public:
		JigsawWorldGenerator();

		void generateWorld(unsigned int seed);

	private:
		std::vector<MapTemplate> templates_;

		std::mt19937_64 eng_ = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());

		void loadTemplates();

		bool tryPlaceTemplate(const MapTemplate &temp, int posX, int posY, bool force = false);
		bool canPlaceTemplate(const MapTemplate &temp, int posX, int posY);

		void balanceObjectType(ObjectType type, int amount);
		void placeWalls();
		void mirrorWorld();
		void varyDepositIncome();
};
