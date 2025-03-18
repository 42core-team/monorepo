#include "DistancedResourceWorldGenerator.h"

DistancedResourceWorldGenerator::DistancedResourceWorldGenerator()
{
}

DistancedResourceWorldGenerator::~DistancedResourceWorldGenerator()
{
}

void DistancedResourceWorldGenerator::generateWorld(Game * game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;
	int resourceCount = 25;

	for (int i = 0; i < resourceCount; i++)
	{
		std::uniform_int_distribution<int> distX(0, width);
		std::uniform_int_distribution<int> distY(0, height);

		Position targetPos(distX(eng_), distY(eng_));

		int adjacentObjects = 0;
		for (int sy = -1; sy <= 1; sy++)
			for (int sx = -1; sx <= 1; sx++)
				if (game->getObjectAtPos(Position(targetPos.x + sx, targetPos.y + sy)) != nullptr)
					adjacentObjects++;

		if (adjacentObjects == 0)
			game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
		else
			i--;
	}
}
