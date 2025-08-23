#include "JigsawWorldGenerator.h"

JigsawWorldGenerator::JigsawWorldGenerator()
{
	loadTemplates();
}

// before mirroring, everything should only be placed in the top-left triangle
// static bool isValidPreMirrorPos(Position pos)
// {
// 	return pos.x + pos.y < Config::game().gridSize - 1;
// }

// ----- STEP 1: Template Placing

void JigsawWorldGenerator::loadTemplates()
{
	const std::string templateFolder = Config::server().dataFolderPath + "/jigsaw-templates";
	std::vector<std::string> files;
	for (auto &e : std::filesystem::directory_iterator(templateFolder))
		if (e.path().extension() == ".txt")
			files.push_back(e.path().string());
	std::sort(files.begin(), files.end());
	for (auto &f : files)
		templates_.emplace_back(f);
	if (templates_.empty())
	{
		Logger::Log(LogLevel::ERROR, "No valid templates found in template folder.");
		exit(EXIT_FAILURE);
	}
}

bool JigsawWorldGenerator::canPlaceTemplate(const MapTemplate &temp, int posX, int posY)
{
	int minSpacing = Config::game().worldGeneratorConfig.value("minTemplateSpacing", 1);
	int minCoreDistance = Config::game().worldGeneratorConfig.value("minCoreDistance", 5);

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

			if (Board::instance().getObjectAtPos(targetPos) != nullptr)
				return false;

			for (int sy = -minSpacing; sy <= minSpacing; ++sy)
			{
				for (int sx = -minSpacing; sx <= minSpacing; ++sx)
				{
					Position neighbor(targetX + sx, targetY + sy);
					if (Board::instance().getObjectAtPos(neighbor) != nullptr)
						return false;
				}
			}

			for (const auto &obj : Board::instance())
				if (obj.getType() == ObjectType::Core && Board::instance().getObjectPositionById(obj.getId()).distance(targetPos) < minCoreDistance)
					return false;
		}
	}
	return true;
}
bool JigsawWorldGenerator::tryPlaceTemplate(const MapTemplate &temp, int posX, int posY, bool force)
{
	if (!canPlaceTemplate(temp, posX, posY) && !force)
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
		{'V', 9}};

	for (int y = 0; y < temp.height; ++y)
	{
		for (int x = 0; x < temp.width; ++x)
		{
			char cell = temp.grid[y][x];
			if (cell == ' ')
				continue;

			if (posX + x < 0 || posX + x >= (int)Config::game().gridSize || posY + y < 0 || posY + y >= (int)Config::game().gridSize)
				continue;

			Position targetPos(posX + x, posY + y);
			if (cell == 'X')
				Board::instance().addObject<Wall>(Wall(), targetPos);
			else if (cell == 'R')
				Board::instance().addObject<Resource>(Resource(), targetPos);
			else if (cell == 'M')
				Board::instance().addObject<Money>(Money(), targetPos);
			else if (std::string("0123456789").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - '0';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					Board::instance().addObject<Wall>(Wall(), targetPos);
			}
			else if (std::string("abcdefghij").find(cell) != std::string::npos)
			{
				int resourceLikelihood = cell - 'a';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < resourceLikelihood)
					Board::instance().addObject<Resource>(Resource(), targetPos);
			}
			else if (std::string("ABCDEFGHIJ").find(cell) != std::string::npos)
			{
				int moneyLikelihood = cell - 'A';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < moneyLikelihood)
					Board::instance().addObject<Money>(Money(), targetPos);
			}
			else if (std::string("klmnopqrst").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - 'k';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					Board::instance().addObject<Wall>(Wall(), targetPos);
				else
					Board::instance().addObject<Resource>(Resource(), targetPos);
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
					Board::instance().addObject<Money>(Money(), targetPos);
				else
					Board::instance().addObject<Wall>(Wall(), targetPos);
			}
			else if (std::string("KLNOPQSTUV").find(cell) != std::string::npos)
			{
				auto it = moneyLikelihoodMap.find(cell);
				if (it != moneyLikelihoodMap.end())
				{
					int moneyLikelihood = it->second;
					std::uniform_int_distribution<int> dist(0, 9);
					if (dist(eng_) < moneyLikelihood)
						Board::instance().addObject<Money>(Money(), targetPos);
					else
						Board::instance().addObject<Resource>(Resource(), targetPos);
				}
			}
		}
	}
	return true;
}

// ----- STEP 2: Walls Placing

void JigsawWorldGenerator::placeWalls()
{
	int additionalWallPlaceAttemptCount = Config::game().worldGeneratorConfig.value("additionalWallPlaceAttemptCount", 100);
	int minCoreDistance = Config::game().worldGeneratorConfig.value("minCoreDistance", 5);

	std::uniform_int_distribution<int> distX(0, Config::game().gridSize - 1);
	std::uniform_int_distribution<int> distY(0, Config::game().gridSize - 1);
	std::uniform_real_distribution<double> probDist(0.0, 1.0);

	for (int i = 0; i < additionalWallPlaceAttemptCount; ++i)
	{
		int x = distX(eng_);
		int y = distY(eng_);
		Position pos(x, y);

		if (Board::instance().getObjectAtPos(pos) != nullptr)
			continue;

		bool nearCore = false;
		for (const Object & obj : Board::instance())
		{
			if (obj.getType() == ObjectType::Core && Board::instance().getObjectPositionById(obj.getId()).distance(pos) < minCoreDistance)
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
				if (Board::instance().getObjectAtPos(neighbor) != nullptr)
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
			Board::instance().addObject<Wall>(Wall(), pos);
	}
}


// ----- STEP 3 + 4: Resource & Money Balancing

// TODO: Make this a template function. the if statement down there is painful
void JigsawWorldGenerator::balanceObjectType(ObjectType type, int amount)
{
	int minCoreDistance = Config::game().worldGeneratorConfig.value("minCoreDistance", 5);

	std::vector<unsigned int> typeObjectIds;
	for (auto & obj : Board::instance())
		if (obj.getType() == type)
			typeObjectIds.push_back(obj.getId());
	int currentObjCount = typeObjectIds.size();

	if (currentObjCount > amount)
	{
		int removeCount = currentObjCount - amount;
		std::shuffle(typeObjectIds.begin(), typeObjectIds.end(), eng_);
		for (unsigned int id : typeObjectIds)
		{
			if (removeCount-- <= 0)
				break;
			Board::instance().removeObjectById(id);
		}
	}
	else if (currentObjCount < amount)
	{
		int addCount = amount - currentObjCount;
		std::uniform_int_distribution<int> distX(0, Config::game().gridSize - 1);
		std::uniform_int_distribution<int> distY(0, Config::game().gridSize - 1);

		int max_iter = 10000;
		while (addCount > 0 && --max_iter > 0)
		{
			int x = distX(eng_);
			int y = distY(eng_);
			Position pos(x, y);

			bool nearCore = false;
			for (const Object & obj : Board::instance())
			{
				if (obj.getType() == ObjectType::Core && Board::instance().getObjectPositionById(obj.getId()).distance(pos) < minCoreDistance)
				{
					nearCore = true;
					break;
				}
			}
			if (nearCore)
				continue;

			// likelihood tweaking based on surroundings for more interesting world gen
			int rerollLikeliness = 100;

			// higher likelihood around walls
			int wallsCount = 0;
			for (int xW = -1; xW <= 1; xW++)
				for (int yW = -1; yW <= 1; yW++)
					if (Board::instance().getObjectAtPos(Position(x + xW, y + yW)) != nullptr && \
						Board::instance().getObjectAtPos(Position(x + xW, y + yW))->getType() == ObjectType::Wall)
						wallsCount++;
			rerollLikeliness -= wallsCount * 5;

			// higher likelihood near non-core corners
			int bottomLeftDistance = pos.distance(Position(0, Config::game().gridSize - 1));
			int topRightDistance = pos.distance(Position(Config::game().gridSize - 1, 0));
			int cornerDist = bottomLeftDistance < topRightDistance ? bottomLeftDistance : topRightDistance;
			rerollLikeliness -= Config::game().gridSize - cornerDist;

			if (rerollLikeliness < 0)
				rerollLikeliness = 0;

			std::uniform_int_distribution<int> pick(0, 100);
			if (pick(eng_) < rerollLikeliness)
				continue ;

			if (Board::instance().getObjectAtPos(pos) == nullptr)
			{
				if (type == ObjectType::Resource)
					Board::instance().addObject<Resource>(Resource(), pos);
				else
					Board::instance().addObject<Money>(Money(), pos);
				addCount--;
			}
		}
	}
}

// ----- STEP 5: Varying Money & Resource Income

void JigsawWorldGenerator::varyResourceIncome()
{
	const int baseResourceIncome = Config::game().worldGeneratorConfig.value("baseResourceIncome", 200);
	const int additionalIncomePerAdjacentWall = Config::game().worldGeneratorConfig.value("resourceAdditionalIncomePerAdjacentWall", 20);
	const float multiplierIfFullySurrounded = Config::game().worldGeneratorConfig.value("resourceMultiplierIfFullySurrounded", 1.3);
	const int randomVariation = Config::game().worldGeneratorConfig.value("randomResourceIncomeVariation", 0);

	for (Object & obj : Board::instance())
	{
		if (obj.getType() != ObjectType::Resource)
			continue;

		Position pos = Board::instance().getObjectPositionById(obj.getId());
		static Position dirs[] = {{1,0},{-1,0},{0,1},{0,-1}};
		auto& board = Board::instance();
		int adjacentWalls = 0;
		for (auto& d : dirs) {
			auto* o = board.getObjectAtPos({pos.x + d.x, pos.y + d.y});
			if (o && (o->getType() == ObjectType::Wall || o->getType() == ObjectType::Resource))
				adjacentWalls++;
		}

		int income = baseResourceIncome + additionalIncomePerAdjacentWall * adjacentWalls;
		if (adjacentWalls >= 4)
			income = static_cast<int>(income * multiplierIfFullySurrounded);

		if (randomVariation > 0)
		{
			int variation = randomVariation / 2;
			std::uniform_int_distribution<int> d(0, randomVariation - 1);
			income = income - variation + d(eng_);
		}

		Resource & ResourceObj = static_cast<Resource &>(obj);
		ResourceObj.setBalance(income);
	}
}

// ----- STEP 6: Mirroring World

void JigsawWorldGenerator::mirrorWorld()
{
	unsigned int W = Config::game().gridSize;
	unsigned int H = Config::game().gridSize;

	for (const Object & obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core)
			continue;
		Position p = Board::instance().getObjectPositionById(obj.getId());
		Position q(W - 1 - p.x, H - 1 - p.y);
		bool isBase =
			(p.y < q.y) ||
			(p.y == q.y && p.x < q.x);
		if (!isBase)
			Board::instance().removeObjectById(obj.getId());
	}
	for (const Object & obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core)
			continue;
		Position p = Board::instance().getObjectPositionById(obj.getId());
		Position q(W - 1 - p.x, H - 1 - p.y);
		if (Board::instance().getObjectAtPos(q) != nullptr)
			continue;
		switch (obj.getType())
		{
			case ObjectType::Wall:
				Board::instance().addObject<Wall>(Wall(), q);
				break;
			case ObjectType::Resource:
				Board::instance().addObject<Resource>(Resource(((Resource &)obj).getBalance()), q);
				break;
			case ObjectType::Money:
				Board::instance().addObject<Money>(Money((Money &)obj).getBalance(), q);
				break;
			default:
				Logger::Log(LogLevel::WARNING, "Unexpected object type during mirroring: " + std::to_string(static_cast<int>(obj.getType())));
		}
	}
}



void JigsawWorldGenerator::generateWorld(unsigned int seed)
{
	eng_ = std::mt19937_64(seed);

	int templatePlaceAttemptCount = Config::game().worldGeneratorConfig.value("templatePlaceAttemptCount", 1000);
	bool mirrorMap = Config::game().worldGeneratorConfig.value("mirrorMap", true);

	Logger::Log("Generating world with JigsawWorldGenerator");

	Visualizer::visualizeGameState(0);

	unsigned int width = Config::game().gridSize;
	unsigned int height = Config::game().gridSize;

	std::uniform_int_distribution<int> distX(0, width + 10);
	std::uniform_int_distribution<int> distY(0, height + 10);
	std::uniform_int_distribution<size_t> templateDist(0, templates_.size() - 1);

	Logger::Log("Step 1: Placing templates");
	for (int i = 0; i < templatePlaceAttemptCount; ++i)
	{
		const MapTemplate &original = templates_[templateDist(eng_)];
		MapTemplate temp = original.getTransformedTemplate(eng_);
		int posX = distX(eng_) - 5;
		int posY = distY(eng_) - 5;
		if (tryPlaceTemplate(temp, posX, posY))
			Logger::Log("Placed template " + original.name + " at (" + std::to_string(posX) + ", " + std::to_string(posY) + ")");
	}
	Visualizer::visualizeGameState(0);

	Logger::Log("Step 2: Placing walls");
	placeWalls();
	Visualizer::visualizeGameState(0);

	Logger::Log("Step 3: Balancing resources");
	balanceObjectType(ObjectType::Resource, Config::game().worldGeneratorConfig.value("resourceCount", 20));
	Visualizer::visualizeGameState(0);

	Logger::Log("Step 4: Balancing moneys");
	balanceObjectType(ObjectType::Money, Config::game().worldGeneratorConfig.value("moneysCount", 20));
	Visualizer::visualizeGameState(0);
	
	Logger::Log("Step 5: Varying Money & Resource Income");
	varyResourceIncome();
	Visualizer::visualizeGameState(0);

	if (mirrorMap)
	{
		Logger::Log("Step 6: Mirroring world");
		mirrorWorld();
		Visualizer::visualizeGameState(0);
	}

	Logger::Log("World generation complete");
}
