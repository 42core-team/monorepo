#pragma once

#include "Board.h"
#include "Config.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Logger.h"
#include "MapTemplate.h"
#include "Object.h"
#include "Visualizer.h"
#include "Wall.h"
#include "WorldGenerator.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdlib>
#include <filesystem>
#include <queue>
#include <random>
#include <string>
#include <vector>

struct Rectangle
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
};

class JigsawWorldGenerator : public WorldGenerator
{
  public:
	JigsawWorldGenerator();

	void generateWorld(uint64_t seed);

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
