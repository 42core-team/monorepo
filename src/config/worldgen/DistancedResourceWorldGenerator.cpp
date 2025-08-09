#include "DistancedResourceWorldGenerator.h"

DistancedResourceWorldGenerator::DistancedResourceWorldGenerator()
{
}

static bool positionHasNeighbours(const Position& pos, int N)
{
	for (int dx = -1; dx <= 1; ++dx)
	{
		for (int dy = -1; dy <= 1; ++dy)
		{
			if (dx == 0 && dy == 0) continue;
			int nx = pos.x + dx;
			int ny = pos.y + dy;
			if (nx < 0 || nx >= N || ny < 0 || ny >= N) continue;
			if (Board::instance().getObjectAtPos(Position(nx, ny))) return true;
		}
	}
	return false;
}

void DistancedResourceWorldGenerator::generateWorld(unsigned int seed)
{
	eng_ = std::mt19937_64(seed);

	unsigned int gridSize = Config::game().gridSize;
	int resourceCount = Config::game().worldGeneratorConfig.value("resourceCount", 20);
	int moneyCount = Config::game().worldGeneratorConfig.value("moneyCount", 10);
	int wallCount = Config::game().worldGeneratorConfig.value("wallCount", 10);
	int coreBuffer = Config::game().worldGeneratorConfig.value("coreBuffer", 3);

	auto withinCoreBuffer = [&](const Position& p) {
		for (const auto& c : Config::game().corePositions)
			if (std::abs(p.x - c.x) + std::abs(p.y - c.y) <= coreBuffer)
				return true;
		return false;
	};

	const int N = static_cast<int>(gridSize);
	auto tryPlace = [&](ObjectType t) -> bool {
		Position p = Position::random(gridSize);
		if (p.x < 0 || p.x >= N || p.y < 0 || p.y >= N) return false;
		if (Board::instance().getObjectAtPos(p)) return false;
		if (positionHasNeighbours(p, N)) return false;
		if (withinCoreBuffer(p)) return false;
		switch (t)
		{
			case ObjectType::Resource:
				Board::instance().addObject<Resource>(Resource(Board::instance().getNextObjectId()), p);
				return true;
			case ObjectType::Money:
				Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId()), p);
				return true;
			case ObjectType::Wall:
				Board::instance().addObject<Wall>(Wall(Board::instance().getNextObjectId()), p);
				return true;
			default:
				return false;
		}
	};

	auto placeK = [&](ObjectType t, int k) {
		int placed = 0;
		int attempts = 0;
		const int maxAttempts = std::max(N * N * 8, k * 16);
		while (placed < k && attempts < maxAttempts)
		{
			attempts++;
			if (tryPlace(t)) placed++;
		}
	};

	placeK(ObjectType::Resource, resourceCount);
	placeK(ObjectType::Money,    moneyCount);
	placeK(ObjectType::Wall,     wallCount);

	std::vector<int> toRemove;
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core) continue;
		Position pos = Board::instance().getObjectPositionById(obj.getId());
		double ratio = static_cast<double>(pos.x) / (gridSize - 1) +
					   static_cast<double>(pos.y) / (gridSize - 1);
		if (ratio >= 1.0) toRemove.push_back(obj.getId());
	}
	for (int id : toRemove) Board::instance().removeObjectById(id);

	std::vector<std::pair<ObjectType, Position>> clones;
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Core) continue;
		Position pos = Board::instance().getObjectPositionById(obj.getId());
		Position newPos(gridSize - 1 - pos.x, gridSize - 1 - pos.y);
		clones.emplace_back(obj.getType(), newPos);
	}

	for (auto &it : clones)
	{
		switch (it.first)
		{
			case ObjectType::Resource:
				Board::instance().addObject<Resource>(Resource(Board::instance().getNextObjectId()), it.second);
				break;
			case ObjectType::Money:
				Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId()), it.second);
				break;
			case ObjectType::Wall:
				Board::instance().addObject<Wall>(Wall(Board::instance().getNextObjectId()), it.second);
				break;
			default:
				Logger::Log("Unknown object type while mirroring: " + std::to_string(static_cast<int>(it.first)));
				break;
		}
	}
}
