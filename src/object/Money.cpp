#include "Money.h"

Money::Money(unsigned int id)
	: Object(id, 1, ObjectType::Money),
	balance_(Config::game().moneyObjIncome) {}
Money::Money(unsigned int id, unsigned int balance)
	: Object(id, 1, ObjectType::Money),
	balance_(balance) {}

void Money::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
