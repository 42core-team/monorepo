#include "Resource.h"

Resource::Resource(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().resourceHp, ObjectType::Resource) {}

void Resource::getMined(Unit * miner)
{
	hp_ -= Config::getInstance().units[miner->getTypeId()].damageResource;

	miner->addBalance(Config::getInstance().resourceIncome * \
		Config::getInstance().units[miner->getTypeId()].damageResource / \
		Config::getInstance().resourceHp);
}

void Resource::tick(unsigned long long tickCount)
{}
