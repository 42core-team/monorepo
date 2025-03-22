#include "DistancedResourceWorldGenerator.h"

DistancedResourceWorldGenerator::DistancedResourceWorldGenerator()
{
}

void DistancedResourceWorldGenerator::generateWorld(Game * game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;
	int resourceCount = Config::getInstance().worldGeneratorConfig.value("resourceOrMoneyCount", 20);

	for (int i = 0; i < resourceCount; i++)
	{
		std::uniform_int_distribution<int> distX(0, width);
		std::uniform_int_distribution<int> distY(0, height);
		std::uniform_int_distribution<int> resourceOrMoney(0, Config::getInstance().worldGeneratorConfig.value("moneyChance", 4));

		Position targetPos(distX(eng_), distY(eng_));

		int adjacentObjects = 0;
		for (int sy = -1; sy <= 1; sy++)
			for (int sx = -1; sx <= 1; sx++)
				if (game->getObjectAtPos(Position(targetPos.x + sx, targetPos.y + sy)) != nullptr)
					adjacentObjects++;

		if (adjacentObjects == 0)
		{
			if (resourceOrMoney(eng_) == 0)
				game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), targetPos));
			else
				game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
		}
		else
			i--;
	}

	game->getObjects().erase(
		std::remove_if(game->getObjects().begin(), game->getObjects().end(),
			[width, height](const std::unique_ptr<Object>& obj) {
				Position pos = obj->getPosition();
				double ratio = static_cast<double>(pos.x) / (width - 1) +
							   static_cast<double>(pos.y) / (height - 1);
				return (ratio >= 1.0 && obj->getType() != ObjectType::Core);
			}),
		game->getObjects().end()
	);

	std::vector<Object*> objectsToMirror;
	for (const auto& obj : game->getObjects())
	{
		Position pos = obj->getPosition();
		double ratio = static_cast<double>(pos.x) / (width - 1) +
					   static_cast<double>(pos.y) / (height - 1);
		if (ratio < 1.0 && obj->getType() != ObjectType::Core)
		{
			objectsToMirror.push_back(obj.get());
		}
	}

	for (auto* obj : objectsToMirror)
	{
		Position pos = obj->getPosition();
		int newX = height - 1 - pos.x;
		int newY = width  - 1 - pos.y;
		Position newPos(newX, newY);
		obj->clone(newPos, game);
	}
}
