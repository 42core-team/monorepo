#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <vector>
#include <memory>

class Game;

class WorldGenerator
{
	public:
		WorldGenerator() = default;
		virtual ~WorldGenerator() = default;
		
		virtual void generateWorld(unsigned int seed) = 0;
};

#endif // WORLD_GENERATOR_H
