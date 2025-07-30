#include "Money.h"

static unsigned int generateBalance()
{
	int baseBalance = Config::game().moneyObjIncome;
	int variation = Config::game().worldGeneratorConfig.value("moneyBalanceVariation", 20);
	int halfVariation = variation / 2;
	int result = baseBalance + (rand() % (variation + 1)) - halfVariation;
	return (result < 0) ? 0 : result;  // Prevent negative money
}

Money::Money(unsigned int id)
	: Object(id, 1, ObjectType::Money),
	balance_(generateBalance()) {}
Money::Money(unsigned int id, unsigned int balance)
	: Object(id, 1, ObjectType::Money),
	balance_(balance) {}

void Money::tick(unsigned long long tickCount)
{
	(void) tickCount;
}
