#ifndef EMPTY_WORLD_GENERATOR_H
#define EMPTY_WORLD_GENERATOR_H

#include "WorldGenerator.h"

class EmptyWorldGenerator : public WorldGenerator
{
	public:
		EmptyWorldGenerator();

		void generateWorld(unsigned int seed);
};

#endif // EMPTY_WORLD_GENERATOR_H
