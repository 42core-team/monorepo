#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <memory>
#include <vector>

class Game;

class WorldGenerator
{
  public:
	WorldGenerator() = default;
	virtual ~WorldGenerator() = default;

	virtual void generateWorld(uint64_t seed) = 0;
};

#endif // WORLD_GENERATOR_H
