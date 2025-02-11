#include "ResourceOnlyWorldGenerator.h"

ResourceOnlyWorldGenerator::ResourceOnlyWorldGenerator()
{
}

ResourceOnlyWorldGenerator::~ResourceOnlyWorldGenerator()
{
}

void ResourceOnlyWorldGenerator::generateWorld(Game * game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;
	int resourceDensity = 3;

	for (unsigned int i = 0; i < width; i++)
		for (unsigned int j = 0; j < height; j++)
			if (rand() % 100 < resourceDensity)
				if (game->getObjectAtPos(Position(i, j)) == nullptr)
					game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), Position(i, j)));
}
