#pragma once

#include "Game.h"
#include "Config.h"
#include "MapTemplate.h"

#include <vector>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <chrono>
#include <random>

class JigsawWorldGenerator {
	public:
		JigsawWorldGenerator();

		void generateWorld(Game* game);

	private:
		std::vector<MapTemplate> templates_;
		int minSpacing_ = 1;
		int coreSafeRange_ = 3;
		int expectedResourceCount_ = 30;

		std::default_random_engine eng_ = std::default_random_engine(std::chrono::system_clock::now().time_since_epoch().count());

		void loadTemplates();

		bool tryPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY);
		bool canPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY);

		void balanceResources(Game* game);
		void placeWalls(Game* game);
};
