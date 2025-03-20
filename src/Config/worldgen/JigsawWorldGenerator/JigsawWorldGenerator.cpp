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

			for (const auto & core : game->getCores())
				if (core.getPosition().distance(targetPos) < minCoreDistance_)
					return false;
		}
	}
	return true;
}
bool JigsawWorldGenerator::tryPlaceTemplate(Game* game, const MapTemplate &temp, int posX, int posY, bool force = false)
{
	if (!canPlaceTemplate(game, temp, posX, posY) && !force)
		return false;

	const std::unordered_map<char, int> moneyLikelihoodMap = {
		{'K', 0},
		{'L', 1},
		{'N', 2},
		{'O', 3},
		{'P', 4},
		{'Q', 5},
		{'S', 6},
		{'T', 7},
		{'U', 8},
		{'V', 9}
	};

	for (int y = 0; y < temp.height; ++y)
	{
		for (int x = 0; x < temp.width; ++x)
		{
			char cell = temp.grid[y][x];
			if (cell == ' ')
				continue;

			if (posX + x < 0 || posX + x >= (int)Config::getInstance().width || posY + y < 0 || posY + y >= (int)Config::getInstance().height)
				continue;

			Position targetPos(posX + x, posY + y);
			if (cell == 'X')
				game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
			else if (cell == 'R')
				game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
			else if (cell == 'M')
				game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), targetPos));
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
			else if (std::string("ABCDEFGHIJ").find(cell) != std::string::npos)
			{
				int moneyLikelihood = cell - 'A';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < moneyLikelihood)
					game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), targetPos));
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
				int moneyLikelihood = cell - 'u';
				if (cell == '!')
					moneyLikelihood = 6;
				else if (cell == '/')
					moneyLikelihood = 7;
				else if (cell == '$')
					moneyLikelihood = 8;
				else if (cell == '%')
					moneyLikelihood = 9;
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < moneyLikelihood)
					game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), targetPos));
				else
					game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), targetPos));
			}
			else if (std::string("KLNOPQSTUV").find(cell) != std::string::npos)
			{
				auto it = moneyLikelihoodMap.find(cell);
				if (it != moneyLikelihoodMap.end())
				{
					int moneyLikelihood = it->second;
					std::uniform_int_distribution<int> dist(0, 9);
					if (dist(eng_) < moneyLikelihood)
						game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), targetPos));
					else
						game->getObjects().push_back(std::make_unique<Resource>(game->getNextObjectId(), targetPos));
				}
			}
		}
	}
	return true;
}

void JigsawWorldGenerator::balanceResources(Game* game)
{
	std::vector<size_t> resourceIndices;
	for (size_t i = 0; i < game->getObjects().size(); ++i)
		if (game->getObjects()[i]->getType() == ObjectType::Resource)
			resourceIndices.push_back(i);
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

		int max_iter = 1000;
		while (addCount > 0 && --max_iter > 0)
		{
			int x = distX(eng_);
			int y = distY(eng_);
			Position pos(x, y);

			for (const auto & core : game->getCores())
				if (core.getPosition().distance(pos) < minCoreDistance_)
					continue;

			if (game->getObjectAtPos(pos) == nullptr)
			{
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
	std::uniform_real_distribution<double> probDist(0.0, 1.0);

	for (int i = 0; i < 75; ++i)
	{
		int x = distX(eng_);
		int y = distY(eng_);
		Position pos(x, y);
		
		if (game->getObjectAtPos(pos) != nullptr)
			continue;

		bool nearCore = false;
		for (const auto & core : game->getCores())
		{
			if (core.getPosition().distance(pos) < minCoreDistance_)
			{
				nearCore = true;
				break;
			}
		}
		if (nearCore)
			continue;

		int adjacentObjects = 0;
		for (int sy = -1; sy <= 1; ++sy)
		{
			for (int sx = -1; sx <= 1; ++sx)
			{
				if (sx == 0 && sy == 0)
					continue;
				Position neighbor(x + sx, y + sy);
				if (game->getObjectAtPos(neighbor) != nullptr)
					adjacentObjects++;
			}
		}

		if (adjacentObjects >= 4)
			continue;

		double placementProbability = 0.5;
		if (adjacentObjects == 0)
			placementProbability = 1;
		else if (adjacentObjects == 1)
			placementProbability = 0.6;
		else if (adjacentObjects == 2)
			placementProbability = 0.9;
		else if (adjacentObjects == 3)
			placementProbability = 0.2;

		if (probDist(eng_) < placementProbability)
			game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), pos));
	}
}

void JigsawWorldGenerator::generateWorld(Game* game)
{
	Logger::Log(LogLevel::INFO, "Generating world with JigsawWorldGenerator");

	game->visualizeGameState(0);

	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;

	std::uniform_int_distribution<int> distX(0, width * 2);
	std::uniform_int_distribution<int> distY(0, height * 2);
	std::uniform_int_distribution<size_t> templateDist(0, templates_.size() - 1);

	Logger::Log(LogLevel::INFO, "Step 1: Placing templates");
	for (int i = 0; i < 1000; ++i)
	{
		const MapTemplate &original = templates_[templateDist(eng_)];
		MapTemplate temp = original.getTransformedTemplate(eng_);
		int posX = distX(eng_) - width / 2;
		int posY = distY(eng_) - height / 2;
		if (tryPlaceTemplate(game, temp, posX, posY))
			std::cout << "Placed a template at (" << posX << ", " << posY << ")\n";
	}
	game->visualizeGameState(0);

	Logger::Log(LogLevel::INFO, "Step 2: Placing walls");
	placeWalls(game);
	game->visualizeGameState(0);

	Logger::Log(LogLevel::INFO, "Step 3: Balancing resources");
	balanceResources(game);
	game->visualizeGameState(0);

	Logger::Log(LogLevel::INFO, "World generation complete");
}
