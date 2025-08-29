#include "SparseWorldGenerator.h"

SparseWorldGenerator::SparseWorldGenerator()
{
}

static bool positionHasNeighbours(const Position &pos, int N)
{
	for (int dx = -1; dx <= 1; ++dx)
	{
		for (int dy = -1; dy <= 1; ++dy)
		{
			if (dx == 0 && dy == 0)
				continue;
			int nx = pos.x + dx;
			int ny = pos.y + dy;
			if (nx < 0 || nx >= N || ny < 0 || ny >= N)
				continue;
			if (Board::instance().getObjectAtPos(Position(nx, ny)))
				return true;
		}
	}
	return false;
}

void SparseWorldGenerator::generateWorld(unsigned int seed)
{
	eng_.seed(seed);

	unsigned int gridSize = Config::game().gridSize;
	int depositCount = Config::game().worldGeneratorConfig.value("depositCount", 20);
	int depositBalanceVariation = Config::game().worldGeneratorConfig.value("depositBalanceVariation", 10);
	int depositIncome = Config::game().depositIncome;
	int gemPileCount = Config::game().worldGeneratorConfig.value("gemPileCount", 10);
	int gemPileBalanceVariation = Config::game().worldGeneratorConfig.value("gemPileBalanceVariation", 5);
	int gemPileIncome = Config::game().gemPileIncome;
	int wallCount = Config::game().worldGeneratorConfig.value("wallCount", 10);
	int coreBuffer = Config::game().worldGeneratorConfig.value("coreBuffer", 3);

	auto withinCoreBuffer = [&](const Position &p)
	{
		for (const auto &c : Config::game().corePositions)
			if (std::abs(p.x - c.x) + std::abs(p.y - c.y) <= coreBuffer)
				return true;
		return false;
	};

	const int N = static_cast<int>(gridSize);
	auto tryPlace = [&](ObjectType t) -> bool
	{
		static std::uniform_int_distribution<int> dist(0, static_cast<int>(gridSize) - 1);
		Position p{dist(eng_), dist(eng_)};
		if (p.x < 0 || p.x >= N || p.y < 0 || p.y >= N)
			return false;
		if (Board::instance().getObjectAtPos(p))
			return false;
		if (positionHasNeighbours(p, N))
			return false;
		if (withinCoreBuffer(p))
			return false;
		if (p.x + p.y >= N - 1)
			return false; // only place in upper left triangle, rest will be mirrored later
		unsigned int randomDepositBalance = depositIncome + std::uniform_int_distribution<>(-depositBalanceVariation, depositBalanceVariation)(eng_);
		unsigned int randomGemPileBalance = gemPileIncome + std::uniform_int_distribution<>(-gemPileBalanceVariation, gemPileBalanceVariation)(eng_);
		switch (t)
		{
		case ObjectType::Deposit:
			Board::instance().addObject<Deposit>(Deposit(randomDepositBalance), p);
			return true;
		case ObjectType::GemPile:
			Board::instance().addObject<GemPile>(GemPile(randomGemPileBalance), p);
			return true;
		case ObjectType::Wall:
			Board::instance().addObject<Wall>(Wall(), p);
			return true;
		default:
			return false;
		}
	};

	auto placeK = [&](ObjectType t, int k)
	{
		int placed = 0;
		int attempts = 0;
		const int maxAttempts = std::max(N * N * 8, k * 16);
		while (placed < k && attempts < maxAttempts)
		{
			attempts++;
			if (tryPlace(t))
				placed++;
		}
	};

	// halfed -> will be mirrored later
	placeK(ObjectType::Deposit, depositCount / 2);
	placeK(ObjectType::GemPile, gemPileCount / 2);
	placeK(ObjectType::Wall, wallCount / 2);

	std::vector<std::tuple<ObjectType, Position, unsigned int>> clones; // type, pos, balance (for deposits and gempiles)
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core)
			continue;
		Position pos = Board::instance().getObjectPositionById(obj.getId());
		Position newPos(gridSize - 1 - pos.x, gridSize - 1 - pos.y);
		unsigned int balance = obj.getType() == ObjectType::Deposit	  ? static_cast<Deposit &>(obj).getBalance()
							   : obj.getType() == ObjectType::GemPile ? static_cast<GemPile &>(obj).getBalance()
																	  : 0;
		clones.emplace_back(obj.getType(), newPos, balance);
	}

	for (auto &it : clones)
	{
		ObjectType type = std::get<0>(it);
		Position pos = std::get<1>(it);
		unsigned int balance = std::get<2>(it);
		switch (type)
		{
		case ObjectType::Deposit:
			Board::instance().addObject<Deposit>(Deposit(balance), pos);
			break;
		case ObjectType::GemPile:
			Board::instance().addObject<GemPile>(GemPile(balance), pos);
			break;
		case ObjectType::Wall:
			Board::instance().addObject<Wall>(Wall(), pos);
			break;
		default:
			Logger::Log("Unknown object type while mirroring: " + std::to_string(static_cast<int>(type)));
			break;
		}
	}
}
