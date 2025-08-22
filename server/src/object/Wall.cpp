#include "Wall.h"

Wall::Wall()
	: Object(Config::game().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
