#include "DistancedResourceWorldGenerator.h"

DistancedResourceWorldGenerator::DistancedResourceWorldGenerator()
{
}

void DistancedResourceWorldGenerator::generateWorld(Game *game)
{
	Logger::Log("Generating world of type distanced resources.");

	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;
	int resourceCount = Config::getInstance().worldGeneratorConfig.value("resourceOrMoneyCount", 20);

	// place objects randomly

	const int maxRepeats = 1000;
	int repeats = 0;
	for (int i = 0; i < resourceCount; i++)
	{
		repeats ++;
		if (repeats > maxRepeats) break;

		Position randPos = Position::random(width, height);

		// verify there aren't any neighbours directly or diagonally to make pathfinding easier

		bool noNeighbours = true;
		for (int x = -1; x >= 1; x++)
		{
			for (int y = -1; y >= 1; y++)
			{
				if (game->board_.getObjectAtPos(Position(x, y)))
				{
					noNeighbours = false;
					break;
				}
			}
			if (!noNeighbours) break;
		}
		if (!noNeighbours) {
			i--;
			continue; // repick another position
		}

		std::uniform_int_distribution<int> resourceOrMoney(0, Config::getInstance().worldGeneratorConfig.value("moneyChance", 4) - 1);
		if (resourceOrMoney(eng_) == 0)
			game->board_.addObject(Money(game->board_.getNextObjectId(), randPos));
			else
			game->board_.addObject(Resource(game->board_.getNextObjectId(), randPos));
	}

	// mirror playing field

	for (auto &obj : game->board_)
	{
		Position pos = obj.getPosition();
		double ratio = static_cast<double>(pos.x) / (width - 1) +
						static_cast<double>(pos.y) / (height - 1);
		if (ratio >= 1.0 && obj.getType() != ObjectType::Core)
			game->board_.removeObjectById(obj.getId());
	}

	for (auto &obj : game->board_)
	{
		if (obj.getType() == ObjectType::Core)
			continue;
		Position pos = obj.getPosition();
		int newX = height - 1 - pos.x;
		int newY = width - 1 - pos.y;
		Position newPos(newX, newY);
		obj.clone(newPos, game);
	}
}
