#ifndef MAZE_WORLD_GENERATOR_H
#define MAZE_WORLD_GENERATOR_H

#include "WorldGenerator.h"

class MazeWorldGenerator : public WorldGenerator
{
	public:
		MazeWorldGenerator();
		~MazeWorldGenerator();

		void generateWorld(Game * game);
};


#endif // MAZE_WORLD_GENERATOR_H
