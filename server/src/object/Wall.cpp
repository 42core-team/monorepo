#include "Wall.h"

Wall::Wall()
	: Object(Config::game().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}

void Wall::damage(Object *attacker, unsigned int damage)
{
	(void) attacker;

	this->setHP(this->getHP() - damage);

	Stats::instance().inc(stat_keys::damage_walls, damage);
}
