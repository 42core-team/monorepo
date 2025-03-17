#include "JigsawWorldGenerator.h"

JigsawWorldGenerator::JigsawWorldGenerator(unsigned int seed)
{
	if (seed == 0)
		seed = std::chrono::system_clock::now().time_since_epoch().count();
	Logger::Log(LogLevel::INFO, "JigsawWorldGenerator seed: " + std::to_string(seed));
	eng_ = std::default_random_engine(seed);
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

			for (const auto &region : coreWallRegions_)
			{
				if ((unsigned int)targetX >= region.x && (unsigned int)targetX < region.x + region.width &&
					(unsigned int)targetY >= region.y && (unsigned int)targetY < region.y + region.height)
					return false;
			}
		}
	}
	return true;
}
bool JigsawWorldGenerator::tryPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY, bool force = false)
{
	if (!canPlaceTemplate(game, temp, posX, posY) && !force)
		return false;

	for (int y = 0; y < temp.height; ++y)
	{
		for (int x = 0; x < temp.width; ++x)
		{
			char cell = temp.grid[y][x];
			if (cell == ' ')
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
			else if (std::string("klmnopqrst").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - 'k';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
				else
					game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
			}
			else if (std::string("uvwxyz!/$%").find(cell) != std::string::npos)
			{
				int resourceLikelihood = cell - 'u';
				if (cell == '!')
					resourceLikelihood = 6;
				else if (cell == '/')
					resourceLikelihood = 7;
				else if (cell == '$')
					resourceLikelihood = 8;
				else if (cell == '%')
					resourceLikelihood = 9;
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < resourceLikelihood)
					game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
				else
					game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
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
		bool canBeRemoved = true;
		if (game->getObjects()[i]->getType() != ObjectType::Resource)
			canBeRemoved = false;
		for (const auto &safeZone : coreWallRegions_)
		{
			if (game->getObjects()[i]->getPosition().x >= safeZone.x &&
				game->getObjects()[i]->getPosition().x < safeZone.x + safeZone.width &&
				game->getObjects()[i]->getPosition().y >= safeZone.y &&
				game->getObjects()[i]->getPosition().y < safeZone.y + safeZone.height)
			{
				canBeRemoved = false;
				break;
			}
		}
		
		if (canBeRemoved)
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
				for (const auto &safeZone : coreWallRegions_)
				{
					if (pos.x >= safeZone.x && pos.x < safeZone.x + safeZone.width &&
						pos.y >= safeZone.y && pos.y < safeZone.y + safeZone.height)
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
			for (const auto &safeZone : coreWallRegions_)
			{
				if (pos.x >= safeZone.x && pos.x < safeZone.x + safeZone.width &&
					pos.y >= safeZone.y && pos.y < safeZone.y + safeZone.height)
				{
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

void JigsawWorldGenerator::placeCoreWalls(Game* game)
{
	std::vector<MapTemplate> coreWallTemplates;
	for (const auto &entry : std::filesystem::directory_iterator("./core/src/Config/worldgen/JigsawWorldGenerator/core_wall_templates")) {
		if (entry.path().extension() == ".txt")
		{
			try {
				coreWallTemplates.emplace_back(entry.path().string());
			}
			catch (const std::exception &e) {
				std::cerr << "Error loading core wall template " << entry.path() << ": " << e.what() << std::endl;
			}
		}
	}
	if (coreWallTemplates.empty())
		throw std::runtime_error("No valid core wall templates found in core_wall_template folder.");

	std::uniform_int_distribution<size_t> dist(0, coreWallTemplates.size() - 1);
	const MapTemplate selectedTemplate = coreWallTemplates[dist(eng_)];

	unsigned int worldWidth = Config::getInstance().width;
	unsigned int worldHeight = Config::getInstance().height;

	for (const auto &core : game->getCores()) {
		Position corePos = core.getPosition();
		bool isLeft = (corePos.x < worldWidth / 2);
		bool isTop  = (corePos.y < worldHeight / 2);
		
		MapTemplate orientedTemplate = selectedTemplate;
		int placeX = 0;
		int placeY = 0;

		if (isTop && isLeft) {
			placeX = 0;
			placeY = 0;
		} else if (isTop && !isLeft) {
			orientedTemplate = orientedTemplate.rotate90();
			placeX = worldWidth - orientedTemplate.width;
			placeY = 0;
		} else if (!isTop && isLeft) {
			orientedTemplate = orientedTemplate.rotate90();
			orientedTemplate = orientedTemplate.rotate90();
			orientedTemplate = orientedTemplate.rotate90();
			placeX = 0;
			placeY = worldHeight - orientedTemplate.height;
		} else if (!isTop && !isLeft) {
			orientedTemplate = orientedTemplate.rotate90();
			orientedTemplate = orientedTemplate.rotate90();
			placeX = worldWidth - orientedTemplate.width;
			placeY = worldHeight - orientedTemplate.height;
		}

		tryPlaceTemplate(game, orientedTemplate, placeX, placeY, true);
		coreWallRegions_.push_back({ (unsigned int)placeX, (unsigned int)placeY, (unsigned int)orientedTemplate.width, (unsigned int)orientedTemplate.height });
	}
}

void JigsawWorldGenerator::generateWorld(Game* game)
{
	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;

	placeCoreWalls(game);

	std::uniform_int_distribution<int> distX(0, width - 1);
	std::uniform_int_distribution<int> distY(0, height - 1);
	std::uniform_int_distribution<size_t> templateDist(0, templates_.size() - 1);
	
	for (int i = 0; i < 1000; ++i)
	{
		const MapTemplate &original = templates_[templateDist(eng_)];
		MapTemplate temp = original.getTransformedTemplate(eng_);
		int posX = distX(eng_);
		int posY = distY(eng_);
		if (tryPlaceTemplate(game, temp, posX, posY))
			std::cout << "Successfully placed a template at (" << posX << ", " << posY << ")\n";
	}

	balanceResources(game);
}
