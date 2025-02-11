#include "Wall.h"

Wall::Wall(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().resourceHp, ObjectType::Resource) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
