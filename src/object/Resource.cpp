#include "Resource.h"

Resource::Resource(unsigned int id)
	: Object(id, Config::instance().resourceHp, ObjectType::Resource),
	  balance_(Config::instance().resourceIncome) {}
Resource::Resource(unsigned int id, unsigned int balance)
	: Object(id, static_cast<unsigned int>(std::round(double(balance) / Config::instance().resourceIncome * Config::instance().resourceHp)),
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
