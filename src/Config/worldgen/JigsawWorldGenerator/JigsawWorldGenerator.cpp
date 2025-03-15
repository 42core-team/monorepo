#include "JigsawWorldGenerator.h"

JigsawWorldGenerator::JigsawWorldGenerator()
{
	loadTemplates();
}

void JigsawWorldGenerator::loadTemplates()
{
	for (const auto & entry : std::filesystem::directory_iterator("./core/src/Config/worldgen/JigsawWorldGenerator/templates")) {
		if (entry.path().extension() == ".txt")
		{
			try
			{
				templates_.emplace_back(entry.path().string());
			}
			catch (const std::exception &e)
			{
				std::cerr << "Error loading template " << entry.path() << ": " << e.what() << std::endl;
			}
		}
	}
	if (templates_.empty())
		throw std::runtime_error("No valid templates found in template folder.");
}

bool JigsawWorldGenerator::canPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY)
{
	for (int y = 0; y < temp.height; ++y)
	{
		for (int x = 0; x < temp.width; ++x)
		{
			char cell = temp.grid[y][x];
			if (cell == ' ')
				continue;

			int targetX = posX + x;
			int targetY = posY + y;
			Position targetPos(targetX, targetY);

			if (targetX < 0 || targetX >= (int)Config::getInstance().width ||
				targetY < 0 || targetY >= (int)Config::getInstance().height)
				return false;

			if (game->getObjectAtPos(targetPos) != nullptr)
				return false;

			for (int sy = -minSpacing_; sy <= minSpacing_; ++sy)
			{
				for (int sx = -minSpacing_; sx <= minSpacing_; ++sx)
				{
					Position neighbor(targetX + sx, targetY + sy);
					if (game->getObjectAtPos(neighbor) != nullptr)
						return false;
				}
			}
		}
	}
	return true;
}
bool JigsawWorldGenerator::tryPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY)
{
	if (!canPlaceTemplate(game, temp, posX, posY))
		return false;

	for (int y = 0; y < temp.height; ++y)
	{
		for (int x = 0; x < temp.width; ++x)
		{
			char cell = temp.grid[y][x];
			if (cell == ' ')
				continue;

			bool tooClose = false;
			for (const auto &core : game->getCores())
			{
				int dx = posX + x - core.getPosition().x;
				int dy = posY + y - core.getPosition().y;
				if (std::sqrt(dx * dx + dy * dy) < coreSafeRange_)
				{
					tooClose = true;
					break;
				}
			}
			if (tooClose)
				continue;

			Position targetPos(posX + x, posY + y);
			if (cell == 'X')
				game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
			else if (cell == 'R')
				game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
			else if (std::string("0123456789").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - '0';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
			}
			else if (std::string("abcdefghij").find(cell) != std::string::npos)
			{
				int resourceLikelihood = cell - 'a';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < resourceLikelihood)
					game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
			}
		}
	}
	return true;
}

void JigsawWorldGenerator::balanceResources(Game* game)
{
	std::vector<size_t> resourceIndices;
	for (size_t i = 0; i < game->getObjects().size(); ++i)
	{
		if (game->getObjects()[i]->getType() == ObjectType::Resource)
			resourceIndices.push_back(i);
	}
	int currentResources = resourceIndices.size();
	
	if (currentResources > expectedResourceCount_)
	{
		int removeCount = currentResources - expectedResourceCount_;

		std::shuffle(resourceIndices.begin(), resourceIndices.end(), eng_);

		std::vector<size_t> indicesToRemove(resourceIndices.begin(), resourceIndices.begin() + removeCount);
		std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<size_t>());

		for (size_t idx : indicesToRemove)
		{
			game->getObjects().erase(game->getObjects().begin() + idx);
		}
	}
	else if (currentResources < expectedResourceCount_)
	{
		int addCount = expectedResourceCount_ - currentResources;
		std::uniform_int_distribution<int> distX(0, Config::getInstance().width - 1);
		std::uniform_int_distribution<int> distY(0, Config::getInstance().height - 1);

		while (addCount > 0)
		{
			int x = distX(eng_);
			int y = distY(eng_);
			Position pos(x, y);

			if (game->getObjectAtPos(pos) == nullptr)
			{
				bool nearCore = false;
				for (const auto &core : game->getCores())
				{
					int dx = x - core.getPosition().x;
					int dy = y - core.getPosition().y;
					if (std::sqrt(dx * dx + dy * dy) < coreSafeRange_)
					{
						nearCore = true;
						break;
					}
				}
				if (nearCore)
					continue;

				game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), pos));
				addCount--;
			}
		}
	}
}

void JigsawWorldGenerator::placeWalls(Game* game)
{
	std::uniform_int_distribution<int> distX(0, Config::getInstance().width - 1);
	std::uniform_int_distribution<int> distY(0, Config::getInstance().height - 1);

	for (int i = 0; i < 100; ++i)
	{
		int x = distX(eng_);
		int y = distY(eng_);
		Position pos(x, y);
		if (game->getObjectAtPos(pos) == nullptr)
		{
			bool nearCore = false;
			for (const auto &core : game->getCores())
			{
				int dx = x - core.getPosition().x;
				int dy = y - core.getPosition().y;
				if (std::sqrt(dx * dx + dy * dy) < coreSafeRange_) {
					nearCore = true;
					break;
				}
			}
			if (nearCore)
				continue;

			bool canPlace = true;
			for (int sy = -1; sy <= 1; ++sy)
			{
				for (int sx = -1; sx <= 1; ++sx)
				{
					Position neighbor(x + sx, y + sy);
					if (game->getObjectAtPos(neighbor) != nullptr)
					{
						canPlace = false;
						break;
					}
				}
				if (!canPlace)
					break;
			}
			if (canPlace)
				game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), pos));
		}
	}
}

void JigsawWorldGenerator::generateWorld(Game* game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;
	std::uniform_int_distribution<int> distX(0, width - 1);
	std::uniform_int_distribution<int> distY(0, height - 1);
	std::uniform_int_distribution<size_t> templateDist(0, templates_.size() - 1);

	for (int i = 0; i < 100; ++i)
	{
		const MapTemplate &original = templates_[templateDist(eng_)];
		MapTemplate temp = original.getTransformedTemplate(eng_);
		int posX = distX(eng_);
		int posY = distY(eng_);
		if (tryPlaceTemplate(game, temp, posX, posY))
			std::cout << "Successfully placed a template " + temp.name + " at (" << posX << ", " << posY << ")\n";
	}

	balanceResources(game);
}
