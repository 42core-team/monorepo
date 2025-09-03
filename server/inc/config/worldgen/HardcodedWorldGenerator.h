#ifndef HARDCODED_WORLD_GENERATOR_H
#define HARDCODED_WORLD_GENERATOR_H

#include "Board.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Logger.h"
#include "Wall.h"
#include "WorldGenerator.h"

#include <random>
#include <string.h>
#include <time.h>

class HardcodedWorldGenerator : public WorldGenerator
{
  public:
	HardcodedWorldGenerator();

	void generateWorld(uint64_t seed);
};

#endif // HARDCODED_WORLD_GENERATOR_H
