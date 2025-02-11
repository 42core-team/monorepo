#include "Wall.h"

Wall::Wall(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
