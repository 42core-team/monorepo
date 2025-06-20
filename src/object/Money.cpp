#include "Money.h"

#include "Game.h"

Money::Money(unsigned int id, Position pos)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(Config::getInstance().resourceIncome) {}
Money::Money(unsigned int id, Position pos, unsigned int balance)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(balance) {}

void Money::tick(unsigned long long tickCount, Game * game)
{
	(void) tickCount;
	(void) game;
}
