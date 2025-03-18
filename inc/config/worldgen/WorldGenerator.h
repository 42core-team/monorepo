#ifndef WORLD_GENERATOR_H
#define WORLD_GENERATOR_H

#include <vector>
#include <memory>

#include "Object.h"
#include "Resource.h"
#include "Wall.h"
#include "Game.h"

class Game;

class WorldGenerator
{
	public:
		WorldGenerator();
		virtual ~WorldGenerator() = default;
		
		virtual void generateWorld(Game * game) = 0;
};

#endif // WORLD_GENERATOR_H
