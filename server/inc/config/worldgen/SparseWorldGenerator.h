#ifndef DISTANCED_RESOURCE_WORLD_GENERATOR_H
#define DISTANCED_RESOURCE_WORLD_GENERATOR_H

#include "WorldGenerator.h"
#include "Board.h"
#include "GemPile.h"
#include "Wall.h"
#include "Deposit.h"
#include "Logger.h"

#include <random>
#include <time.h>
#include <string.h>

class SparseWorldGenerator : public WorldGenerator
{
	public:
		SparseWorldGenerator();

		void generateWorld(unsigned int seed);

	private:
		std::mt19937_64 eng_ = std::mt19937_64(std::chrono::system_clock::now().time_since_epoch().count());
};

#endif // DISTANCED_RESOURCE_WORLD_GENERATOR_H
