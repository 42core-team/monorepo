#include "Wall.h"

Wall::Wall(unsigned int id)
	: Object(id, Config::instance().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
