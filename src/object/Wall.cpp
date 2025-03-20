#include "Wall.h"

#include "Game.h"

Wall::Wall(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().wallHp, ObjectType::Wall) {}

void Wall::tick(unsigned long long tickCount)
{
	(void) tickCount;
}

std::unique_ptr<Object> & Wall::clone(Position newPos, Game * game) const
{
	int nextObjId = game->getNextObjectId();
	std::unique_ptr<Object> obj = std::make_unique<Wall>(nextObjId, newPos);
	obj->setHP(this->getHP());
	game->getObjects().push_back(std::move(obj));
	return game->getObjects().back();
}
