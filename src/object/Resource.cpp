#include "Resource.h"

Resource::Resource(unsigned int id)
	: Object(id, Config::game().resourceHp, ObjectType::Resource),
	  balance_(Config::game().resourceIncome) {}
Resource::Resource(unsigned int id, unsigned int balance)
	: Object(id, static_cast<unsigned int>(std::round(double(balance) / Config::game().resourceIncome * Config::game().resourceHp)),
			 ObjectType::Resource),
	  balance_(balance) {}

void Resource::getMined(Unit *miner)
{
	miner->addBalance(balance_);
}

void Resource::tick(unsigned long long tickCount)
{
	(void)tickCount;
}
