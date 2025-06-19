#include "JigsawWorldGenerator.h"

JigsawWorldGenerator::JigsawWorldGenerator(unsigned int seed)
{
	if (seed == 0)
		seed = std::chrono::system_clock::now().time_since_epoch().count();
	Logger::Log("JigsawWorldGenerator seed: " + std::to_string(seed));
	eng_ = std::default_random_engine(seed);
	loadTemplates();
}

void JigsawWorldGenerator::loadTemplates()
{
	for (const auto &entry : std::filesystem::directory_iterator("./data/jigsaw-templates/"))
	{
		if (entry.path().extension() != ".txt")
		{
			Logger::Log(LogLevel::WARNING, "Skipping non-txt file: " + entry.path().string());
			continue;
		}
		try
		{
			templates_.emplace_back(entry.path().string());
		}
		catch (const std::exception &e)
		{
			Logger::Log(LogLevel::ERROR, "Error loading template " + entry.path().string() + ": " + e.what());
		}
	}
	if (templates_.empty())
	{
		Logger::Log(LogLevel::ERROR, "No valid templates found in template folder.");
		exit(EXIT_FAILURE);
	}
}

bool JigsawWorldGenerator::canPlaceTemplate(Game *game, const MapTemplate &temp, int posX, int posY)
{
	int minSpacing = Config::getInstance().worldGeneratorConfig.value("minTemplateSpacing", 1);
	int minCoreDistance = Config::getInstance().worldGeneratorConfig.value("minCoreDistance", 5);

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

			if (game->board_.getObjectAtPos(targetPos) != nullptr)
				return false;

			for (int sy = -minSpacing; sy <= minSpacing; ++sy)
			{
				for (int sx = -minSpacing; sx <= minSpacing; ++sx)
				{
					Position neighbor(targetX + sx, targetY + sy);
					if (game->board_.getObjectAtPos(neighbor) != nullptr)
						return false;
				}
			}

			for (const auto &obj : game->board_)
				if (obj.getType() == ObjectType::Core && ((Core &)obj).getPosition().distance(targetPos) < minCoreDistance)
					return false;
		}
	}
	return true;
}
bool JigsawWorldGenerator::tryPlaceTemplate(Game *game, const MapTemplate &temp, int posX, int posY, bool force = false)
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
		{'V', 9}};

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
				game->board_.addObject(Wall(game->board_.getNextObjectId(), targetPos));
			else if (cell == 'R')
				game->board_.addObject(Resource(game->board_.getNextObjectId(), targetPos));
			else if (cell == 'M')
				game->board_.addObject(Money(game->board_.getNextObjectId(), targetPos));
			else if (std::string("0123456789").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - '0';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					game->board_.addObject(Wall(game->board_.getNextObjectId(), targetPos));
			}
			else if (std::string("abcdefghij").find(cell) != std::string::npos)
			{
				int resourceLikelihood = cell - 'a';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < resourceLikelihood)
					game->board_.addObject(Resource(game->board_.getNextObjectId(), targetPos));
			}
			else if (std::string("ABCDEFGHIJ").find(cell) != std::string::npos)
			{
				int moneyLikelihood = cell - 'A';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < moneyLikelihood)
					game->board_.addObject(Money(game->board_.getNextObjectId(), targetPos));
			}
			else if (std::string("klmnopqrst").find(cell) != std::string::npos)
			{
				int wallLikelihood = cell - 'k';
				std::uniform_int_distribution<int> dist(0, 9);
				if (dist(eng_) < wallLikelihood)
					game->board_.addObject(Wall(game->board_.getNextObjectId(), targetPos));
				else
					game->board_.addObject(Resource(game->board_.getNextObjectId(), targetPos));
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
					game->board_.addObject(Money(game->board_.getNextObjectId(), targetPos));
				else
					game->board_.addObject(Wall(game->board_.getNextObjectId(), targetPos));
			}
			else if (std::string("KLNOPQSTUV").find(cell) != std::string::npos)
			{
				auto it = moneyLikelihoodMap.find(cell);
				if (it != moneyLikelihoodMap.end())
				{
					int moneyLikelihood = it->second;
					std::uniform_int_distribution<int> dist(0, 9);
					if (dist(eng_) < moneyLikelihood)
						game->board_.addObject(Money(game->board_.getNextObjectId(), targetPos));
					else
						game->board_.addObject(Resource(game->board_.getNextObjectId(), targetPos));
				}
			}
		}
	}
	return true;
}

// TODO: Make this a template function. the if statement down there is painful
void JigsawWorldGenerator::balanceObjectType(Game *game, ObjectType type, int amount)
{
	int minCoreDistance = Config::getInstance().worldGeneratorConfig.value("minCoreDistance", 5);

	std::vector<Object &> typeObjects;
	for (auto & obj : game->board_)
		if (obj.getType() == type)
		typeObjects.push_back(obj);
	int currentObjCount = typeObjects.size();

	if (currentObjCount > amount)
	{
		int removeCount = currentObjCount - amount;
		std::shuffle(typeObjects.begin(), typeObjects.end(), eng_);
		for (Object & obj : typeObjects)
			game->board_.removeObjectById(obj.getId());
	}
	else if (currentObjCount < amount)
	{
		int addCount = amount - currentObjCount;
		std::uniform_int_distribution<int> distX(0, Config::getInstance().width - 1);
		std::uniform_int_distribution<int> distY(0, Config::getInstance().height - 1);

		int max_iter = 10000;
		while (addCount > 0 && --max_iter > 0)
		{
			int x = distX(eng_);
			int y = distY(eng_);
			Position pos(x, y);

			bool nearCore = false;
			for (const Object & obj : game->board_)
			{
				if (obj.getType() == ObjectType::Core && ((Core &)obj).getPosition().distance(pos) < minCoreDistance)
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
					if (game->board_.getObjectAtPos(Position(x + xW, y + yW)) != nullptr && \
						game->board_.getObjectAtPos(Position(x + xW, y + yW))->getType() == ObjectType::Wall)
						wallsCount++;
			rerollLikeliness -= wallsCount * 5;

			// higher likelihood near non-core corners
			int bottomLeftDistance = pos.distance(Position(0, Config::getInstance().height));
			int topRightDistance = pos.distance(Position(Config::getInstance().width, 0));
			int cornerDist = bottomLeftDistance < topRightDistance ? bottomLeftDistance : topRightDistance;
			rerollLikeliness -= Config::getInstance().width - cornerDist;

			if (rerollLikeliness < 0)
				rerollLikeliness = 0;

			std::uniform_int_distribution<int> pick(0, 100);
			if (pick(eng_) < rerollLikeliness)
				continue ;

			if (game->board_.getObjectAtPos(pos) == nullptr)
			{
				if (type == ObjectType::Resource)
					game->board_.addObject(Resource(game->board_.getNextObjectId(), pos));
				else
					game->board_.addObject(Money(game->board_.getNextObjectId(), pos));
				addCount--;
			}
		}
	}
}

void JigsawWorldGenerator::clearPathBetweenCores(Game *game)
{
	Position start;
	Position end;

	unsigned short coreNbr = 0;
	for (const Object & obj : game->board_)
	{
		if (obj.getType() != ObjectType::Core)
			continue;
		if (coreNbr == 0)
			start = obj.getPosition();
		else
			end = obj.getPosition();
	}

	int W = Config::getInstance().width;
	int H = Config::getInstance().height;

	auto getCost = [game](int x, int y) -> int
	{
		Position pos(x, y);
		Object *obj = game->board_.getObjectAtPos(pos);
		return (obj == nullptr || obj->getType() == ObjectType::Core) ? 0 : 1;
	};

	std::vector<int> dist(W * H, INT_MAX);
	std::vector<std::pair<int, int>> prev(W * H, {-1, -1});
	auto index = [W](int x, int y) -> int
	{ return y * W + x; };

	struct Node
	{
		int x, y, cost;
	};
	auto cmp = [](const Node &a, const Node &b)
	{ return a.cost > b.cost; };
	std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);

	dist[index(start.x, start.y)] = 0;
	pq.push({start.x, start.y, 0});

	std::vector<std::pair<int, int>> directions = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

	bool found = false;
	while (!pq.empty())
	{
		Node cur = pq.top();
		pq.pop();

		if (cur.x == end.x && cur.y == end.y)
		{
			found = true;
			break;
		}

		if (cur.cost > dist[index(cur.x, cur.y)])
			continue;

		for (const auto &d : directions)
		{
			int nx = cur.x + d.first;
			int ny = cur.y + d.second;
			if (nx < 0 || ny < 0 || nx >= W || ny >= H)
				continue;
			int cost = getCost(nx, ny);
			int newCost = cur.cost + cost;
			if (newCost < dist[index(nx, ny)])
			{
				dist[index(nx, ny)] = newCost;
				prev[index(nx, ny)] = {cur.x, cur.y};
				pq.push({nx, ny, newCost});
			}
		}
	}

	if (!found)
	{
		Logger::Log(LogLevel::ERROR, "No viable path found between cores.");
		return;
	}

	std::vector<Position> path;
	int cx = end.x, cy = end.y;
	while (!(cx == start.x && cy == start.y))
	{
		path.push_back(Position(cx, cy));
		auto p = prev[index(cx, cy)];
		cx = p.first;
		cy = p.second;
	}
	path.push_back(start);
	std::reverse(path.begin(), path.end());

	for (const auto &pos : path)
	{
		for (const Object & obj : game->board_)
			if (obj.getPosition() == pos && obj.getType() != ObjectType::Core)
				game->board_.removeObjectById(obj.getId());

		if (!Config::getInstance().worldGeneratorConfig.value("mirrorMap", true))
			continue;

		Position mirrorPos((W - 1) - pos.x, (H - 1) - pos.y);
		for (const Object & obj : game->board_)
			if (obj.getPosition() == mirrorPos && obj.getType() != ObjectType::Core)
				game->board_.removeObjectById(obj.getId());
	}
}

void JigsawWorldGenerator::placeWalls(Game *game)
{
	int additionalWallPlaceAttemptCount = Config::getInstance().worldGeneratorConfig.value("additionalWallPlaceAttemptCount", 100);
	int minCoreDistance = Config::getInstance().worldGeneratorConfig.value("minCoreDistance", 5);

	std::uniform_int_distribution<int> distX(0, Config::getInstance().width - 1);
	std::uniform_int_distribution<int> distY(0, Config::getInstance().height - 1);
	std::uniform_real_distribution<double> probDist(0.0, 1.0);

	for (int i = 0; i < additionalWallPlaceAttemptCount; ++i)
	{
		int x = distX(eng_);
		int y = distY(eng_);
		Position pos(x, y);

		if (game->board_.getObjectAtPos(pos) != nullptr)
			continue;

		bool nearCore = false;
		for (const Object & obj : game->board_)
		{
			if (obj.getType() == ObjectType::Core && ((Core &)obj).getPosition().distance(pos) < minCoreDistance)
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
				if (game->board_.getObjectAtPos(neighbor) != nullptr)
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
			game->board_.addObject(Wall(game->board_.getNextObjectId(), pos));
	}
}

void JigsawWorldGenerator::mirrorWorld(Game *game)
{
	unsigned int W = Config::getInstance().width;
	unsigned int H = Config::getInstance().height;

	std::vector<Object &> base;

	for (Object & obj : game->board_)
	{
		if (obj.getType() == ObjectType::Core)
			continue; // never mirror cores

		const Position p = obj.getPosition();
		const Position q(W - 1 - p.x, H - 1 - p.y);

		// lexicographic compare: pick the 'smaller' of p and q
		bool isBase = (p.y < q.y) ||
					  (p.y == q.y && p.x < q.x);
		if (isBase)
			base.push_back(obj);
	}

	// 2) Erase every non-core object not in the base set
	for (const Object & obj : game->board_)
	{
		Position p = obj.getPosition();
		Position q(W - 1 - p.x, H - 1 - p.y);
		bool isCore = obj.getType() == ObjectType::Core;
		bool isBase =
			(p.y < q.y) ||
			(p.y == q.y && p.x < q.x);
		if (!isCore && !isBase)
			game->board_.removeObjectById(obj.getId());
	}

	// 3) Clone each base object into its 180° partner
	for (Object *o : base)
	{
		Position p = o->getPosition();
		Position mirrorPos(W - 1 - p.x, H - 1 - p.y);
		o->clone(mirrorPos, game);
	}
}

void JigsawWorldGenerator::generateWorld(Game *game)
{
	int templatePlaceAttemptCount = Config::getInstance().worldGeneratorConfig.value("templatePlaceAttemptCount", 1000);
	bool mirrorMap = Config::getInstance().worldGeneratorConfig.value("mirrorMap", true);

	Logger::Log("Generating world with JigsawWorldGenerator");

	game->visualizeGameState(0);

	unsigned int width = Config::getInstance().width;
	unsigned int height = Config::getInstance().height;

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
		if (tryPlaceTemplate(game, temp, posX, posY))
			Logger::Log("Placed template " + original.name + " at (" + std::to_string(posX) + ", " + std::to_string(posY) + ")");
	}
	game->visualizeGameState(0);

	Logger::Log("Step 2: Placing walls");
	placeWalls(game);
	game->visualizeGameState(0);

	Logger::Log("Step 3: Balancing resources");
	balanceObjectType(game, ObjectType::Resource, Config::getInstance().worldGeneratorConfig.value("resourceCount", 20));
	game->visualizeGameState(0);

	Logger::Log("Step 4: Balancing moneys");
	balanceObjectType(game, ObjectType::Money, Config::getInstance().worldGeneratorConfig.value("moneysCount", 20));
	game->visualizeGameState(0);

	if (mirrorMap)
	{
		Logger::Log("Step 5: Mirroring world");
		mirrorWorld(game);
		game->visualizeGameState(0);
	}

	Logger::Log("Step 6: Clearing at least 1 path between cores.");
	clearPathBetweenCores(game);
	game->visualizeGameState(0);

	Logger::Log("World generation complete");
}
