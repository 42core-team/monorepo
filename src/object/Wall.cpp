#include "Wall.h"

#include "Game.h"

Wall::Wall(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount, Game * game)
{
	(void) tickCount;
	(void) game;
}
