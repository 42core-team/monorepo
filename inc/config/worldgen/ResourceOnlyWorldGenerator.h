#ifndef RESOURCE_ONLY_WORLD_GENERATOR_H
#define RESOURCE_ONLY_WORLD_GENERATOR_H

#include "WorldGenerator.h"

class ResourceOnlyWorldGenerator : public WorldGenerator
{
	public:
		ResourceOnlyWorldGenerator();
		~ResourceOnlyWorldGenerator();

		void generateWorld(Game * game);
};


#endif // RESOURCE_ONLY_WORLD_GENERATOR_H
