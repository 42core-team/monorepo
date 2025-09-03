#ifndef DISTANCED_RESOURCE_WORLD_GENERATOR_H
#define DISTANCED_RESOURCE_WORLD_GENERATOR_H

#include "Board.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Logger.h"
#include "Wall.h"
#include "WorldGenerator.h"

#include <random>
#include <string.h>
#include <time.h>

class SparseWorldGenerator : public WorldGenerator
{
  public:
	SparseWorldGenerator();

	void generateWorld(uint64_t seed);

  private:
	std::mt19937_64 eng_{};
};

#endif // DISTANCED_RESOURCE_WORLD_GENERATOR_H
