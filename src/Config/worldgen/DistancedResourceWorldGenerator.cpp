#include "DistancedResourceWorldGenerator.h"

DistancedResourceWorldGenerator::DistancedResourceWorldGenerator()
{
}

void DistancedResourceWorldGenerator::generateWorld()
{
	Logger::Log("Generating world of type distanced resources.");

	unsigned int width = Config::instance().width;
	unsigned int height = Config::instance().height;
	int resourceCount = Config::instance().worldGeneratorConfig.value("resourceOrMoneyCount", 20);

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
				if (Board::instance().getObjectAtPos(Position(x, y)))
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

		std::uniform_int_distribution<int> resourceOrMoney(0, Config::instance().worldGeneratorConfig.value("moneyChance", 4) - 1);
		if (resourceOrMoney(eng_) == 0)
			Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId(), randPos));
		else
			Board::instance().addObject<Resource>(Resource(Board::instance().getNextObjectId(), randPos));
	}

	// mirror playing field

	for (auto &obj : Board::instance())
	{
		Position pos = obj.getPosition();
		double ratio = static_cast<double>(pos.x) / (width - 1) +
						static_cast<double>(pos.y) / (height - 1);
		if (ratio >= 1.0 && obj.getType() != ObjectType::Core)
			Board::instance().removeObjectById(obj.getId());
	}

	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core)
			continue;
		Position pos = obj.getPosition();
		Position newPos(height - 1 - pos.x, width - 1 - pos.y);
		switch (obj.getType())
		{
			case ObjectType::Resource:
				Board::instance().addObject<Resource>(Resource(obj.getId(), newPos));
				break;
			case ObjectType::Money:
				Board::instance().addObject<Money>(Money(obj.getId(), newPos));
				break;
			default:
				Logger::Log("Unknown object type while mirroring: " + std::to_string(static_cast<int>(obj.getType())));
				break;
		}
	}
}
