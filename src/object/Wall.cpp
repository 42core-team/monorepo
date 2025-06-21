#include "Wall.h"

Wall::Wall(unsigned int id, Position pos)
	: Object(id, pos, Config::instance().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
