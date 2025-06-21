#include "Resource.h"

Resource::Resource(unsigned int id, Position pos)
	: Object(id, pos, Config::instance().resourceHp, ObjectType::Resource),
	  balance_(Config::instance().resourceIncome) {}
Resource::Resource(unsigned int id, Position pos, unsigned int balance)
	: Object(id, pos, static_cast<unsigned int>(std::round(double(balance) / Config::instance().resourceIncome * Config::instance().resourceHp)),
			 ObjectType::Resource),
	  balance_(balance) {}

void Resource::getMined(Unit *miner)
{
	unsigned int damage = Config::instance().units[miner->getUnitType()].damageResource;
	if ((int)damage > hp_)
		damage = hp_;

	unsigned int reward = 0;
	if (hp_ > 0)
	{
		if ((int)damage == hp_)
			reward = balance_;
		else
		{
			reward = static_cast<unsigned int>(std::round(double(damage) * double(balance_) / double(hp_)));
			reward = std::min(reward, balance_);
		}
	}

	hp_ -= damage;
	balance_ -= reward;

	miner->addBalance(reward);
}

void Resource::tick(unsigned long long tickCount)
{
	(void)tickCount;
}
