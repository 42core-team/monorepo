#include "Money.h"

#include "Game.h"

Money::Money(unsigned int id, Position pos)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(Config::getInstance().resourceIncome) {}
Money::Money(unsigned int id, Position pos, unsigned int balance)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(balance) {}

void Money::tick(unsigned long long tickCount)
{
	(void) tickCount;
}

std::unique_ptr<Object> & Money::clone(Position newPos, Game * game) const
{
	int nextObjId = game->getNextObjectId();
	std::unique_ptr<Object> obj = std::make_unique<Money>(nextObjId, newPos, balance_);
	obj->setHP(this->getHP());
	game->getObjects().push_back(std::move(obj));
	return game->getObjects().back();
}
