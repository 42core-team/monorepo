#ifndef HARDCODED_WORLD_GENERATOR_H
#define HARDCODED_WORLD_GENERATOR_H

#include "WorldGenerator.h"
#include "Board.h"
#include "GemPile.h"
#include "Wall.h"
#include "Deposit.h"
#include "Logger.h"

#include <random>
#include <time.h>
#include <string.h>

class HardcodedWorldGenerator : public WorldGenerator
{
	public:
		HardcodedWorldGenerator();

		void generateWorld(unsigned int seed);
};

#endif // HARDCODED_WORLD_GENERATOR_H
