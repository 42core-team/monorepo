#ifndef DISTANCED_RESOURCE_WORLD_GENERATOR_H
#define DISTANCED_RESOURCE_WORLD_GENERATOR_H

#include "WorldGenerator.h"
#include "Board.h"
#include "Money.h"
#include "Resource.h"
#include "Logger.h"

#include <random>
#include <time.h>
#include <string.h>

class DistancedResourceWorldGenerator : public WorldGenerator
{
	public:
		DistancedResourceWorldGenerator();

		void generateWorld();

	private:
		std::default_random_engine eng_ = std::default_random_engine(time(nullptr));
};


#endif // DISTANCED_RESOURCE_WORLD_GENERATOR_H
