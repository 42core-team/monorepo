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

		std::cout << "attempting placement at " << randPos.x << " " << randPos.y << std::endl;

		// verify there aren't any neighbours directly or diagonally to make pathfinding easier

		bool noNeighbours = true;
		for (int x = -1; x >= 1; x++)
		{
			for (int y = -1; y >= 1; y++)
			{
				if (game->getObjectAtPos(Position(x, y)))
				{
					noNeighbours = false;
					break;
				}
			}
			if (!noNeighbours) break;
		}
		if (!noNeighbours) {
			std::cout << "whoops, had neighbours. next!" << std::endl;
			i--;
			continue; // repick another position
		}

		std::uniform_int_distribution<int> resourceOrMoney(0, Config::getInstance().worldGeneratorConfig.value("moneyChance", 4) - 1);
		if (resourceOrMoney(eng_) == 0)
			game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), randPos));
		else
			game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), randPos));
	}

	// mirror playing field

	game->getObjects().erase(
		std::remove_if(game->getObjects().begin(), game->getObjects().end(),
					   [width, height](const std::unique_ptr<Object> &obj)
					   {
						   Position pos = obj->getPosition();
						   double ratio = static_cast<double>(pos.x) / (width - 1) +
										  static_cast<double>(pos.y) / (height - 1);
						   return (ratio >= 1.0 && obj->getType() != ObjectType::Core);
					   }),
		game->getObjects().end());

	std::vector<Object *> objectsToMirror;
	for (const auto &obj : game->getObjects())
	{
		Position pos = obj->getPosition();
		double ratio = static_cast<double>(pos.x) / (width - 1) +
						static_cast<double>(pos.y) / (height - 1);
		if (ratio < 1.0 && obj->getType() != ObjectType::Core)
		{
			objectsToMirror.push_back(obj.get());
		}
	}

	for (auto *obj : objectsToMirror)
	{
		Position pos = obj->getPosition();
		int newX = height - 1 - pos.x;
		int newY = width - 1 - pos.y;
		Position newPos(newX, newY);
		obj->clone(newPos, game);
	}
}
