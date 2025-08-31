#include "Bomb.h"

#include <unordered_set>

Bomb::Bomb()
	: Object(1, ObjectType::Bomb), countdown_(Config::game().bombCountdown) {}

void Bomb::explode()
{
	const int radius = Config::game().bombReach;
	const Position bomb = Board::instance().getObjectPositionById(this->getId());

	explosionTiles_.clear();
	explosionTiles_.emplace(bomb);

	auto isPositionExplosionBlocking = [](int x, int y) -> bool {
		if (Object* o = Board::instance().getObjectAtPos(Position{x, y})) return o->getType() == ObjectType::Wall;
		return false;
	};

	auto visitRayFromCenters = [&](int tx, int ty, auto&& onCell) {
		double ox = bomb.x + 0.5, oy = bomb.y + 0.5;
		double dx = (tx + 0.5) - ox, dy = (ty + 0.5) - oy;
		int stepX = (dx > 0) - (dx < 0), stepY = (dy > 0) - (dy < 0);
		double invAbsDx = (dx != 0.0) ? 1.0 / std::fabs(dx) : std::numeric_limits<double>::infinity();
		double invAbsDy = (dy != 0.0) ? 1.0 / std::fabs(dy) : std::numeric_limits<double>::infinity();

		int x = bomb.x, y = bomb.y;
		double tMaxX = (stepX > 0) ? ((std::floor(ox) + 1.0) - ox) * invAbsDx : (ox - std::floor(ox)) * invAbsDx;
		double tMaxY = (stepY > 0) ? ((std::floor(oy) + 1.0) - oy) * invAbsDy : (oy - std::floor(oy)) * invAbsDy;
		double tDeltaX = 1.0 * invAbsDx;
		double tDeltaY = 1.0 * invAbsDy;

		while (!(x == tx && y == ty)) {
			if (tMaxX < tMaxY) { x += stepX; tMaxX += tDeltaX; }
			else               { y += stepY; tMaxY += tDeltaY; }
			if (!onCell(x, y)) break;
		}
	};

	std::unordered_set<unsigned long long> toDamageIds;
	std::vector<Object*> toDamage;

	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			if (Position(bomb.x + dx, bomb.y + dy).distance(bomb) > radius) continue;

			if (dx == 0 && dy == 0) continue;
			int tx = bomb.x + dx, ty = bomb.y + dy;

			visitRayFromCenters(tx, ty, [&](int cx, int cy) {
				if (isPositionExplosionBlocking(cx, cy)) return false;
				if (Object* o = Board::instance().getObjectAtPos(Position{cx, cy})) {
					if (o->getType() != ObjectType::Wall) {
						auto id = o->getId();
						if (toDamageIds.insert(id).second) toDamage.push_back(o);
					}
				}
				explosionTiles_.emplace(Position{cx, cy});
				return true;
			});
		}
	}

	for (Object* o : toDamage)
	{
		if (o->getType() == ObjectType::Core)
			o->setHP(o->getHP() - Config::game().bombDamageCore);
		else if (o->getType() == ObjectType::Unit)
			o->setHP(o->getHP() - Config::game().bombDamageUnit);
		else if (o->getType() == ObjectType::Deposit)
			o->setHP(o->getHP() - Config::game().bombDamageDeposit);
		else if (o->getType() == ObjectType::Bomb)
			((Bomb *)o)->handleAttack(); // trigger chain explosions
	}

	this->hp_ = 0;
}

void Bomb::tick(unsigned long long tickCount)
{
	(void)tickCount;
	if (!countdownStarted_)
		return;
	if (countdown_ > 0)
		countdown_--;
	else
		explode();
}

void Bomb::handleAttack()
{
	if (!countdownStarted_)
		countdownStarted_ = true;
	else
		countdown_ = 0;
}
