#include "MazeWorldGenerator.h"

MazeWorldGenerator::MazeWorldGenerator()
{
}

MazeWorldGenerator::~MazeWorldGenerator()
{
}

static void recurseCreateMaze(Game* game, const Position & pos)
{
	if (game->getObjectAtPos(pos) != nullptr)
		return;

	if (game->getObjectAtPos(pos) == nullptr)
		game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), pos));

	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;

	std::vector<Position> directions = {
		Position(0, -1),
		Position(1, 0),
		Position(0, 1),
		Position(-1, 0)
	};

	shuffle_vector(directions);

	for (const auto & offset : directions)
	{
		Position next(pos.x + 2 * offset.x, pos.y + 2 * offset.y);
		Position between(pos.x + offset.x, pos.y + offset.y);

		if ((int)next.x < 0 || next.x >= width || (int)next.y < 0 || next.y >= height)
			continue;

		if (game->getObjectAtPos(next) == nullptr)
		{
			if (game->getObjectAtPos(between) == nullptr)
				game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), between));
			recurseCreateMaze(game, next);
		}
	}
}

void MazeWorldGenerator::generateWorld(Game * game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;

	int startX = rand() % width;
	int startY = rand() % height;
	Position start(startX, startY);

	recurseCreateMaze(game, start);
}
