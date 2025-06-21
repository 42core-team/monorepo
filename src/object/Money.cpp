#include "Money.h"

Money::Money(unsigned int id, Position pos)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(Config::instance().resourceIncome) {}
Money::Money(unsigned int id, Position pos, unsigned int balance)
	: Object(id, pos, 1, ObjectType::Money),
	balance_(balance) {}

void Money::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
