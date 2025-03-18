#ifndef DISTANCED_RESOURCE_WORLD_GENERATOR_H
#define DISTANCED_RESOURCE_WORLD_GENERATOR_H

#include "WorldGenerator.h"

#include <random>
#include <time.h>

#include "Game.h"

class DistancedResourceWorldGenerator : public WorldGenerator
{
	public:
		DistancedResourceWorldGenerator();
		~DistancedResourceWorldGenerator();

		void generateWorld(Game * game);

	private:
		std::default_random_engine eng_ = std::default_random_engine(time(nullptr));
};


#endif // DISTANCED_RESOURCE_WORLD_GENERATOR_H
