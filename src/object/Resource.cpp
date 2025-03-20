#include "Resource.h"

#include "Game.h"

Resource::Resource(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().resourceHp, ObjectType::Resource),
	balance_(Config::getInstance().resourceIncome) {}
Resource::Resource(unsigned int id, Position pos, unsigned int balance)
	: Object(id, pos, static_cast<unsigned int>( std::round( double(balance) / 
		Config::getInstance().resourceIncome * Config::getInstance().resourceHp ) ),
		ObjectType::Resource),
	balance_(balance) {}

void Resource::getMined(Unit * miner)
{
	unsigned int damage = Config::getInstance().units[miner->getTypeId()].damageResource;
	if ((int)damage > hp_)
		damage = hp_;

	unsigned int reward = 0;
	if (hp_ > 0)
	{
		if ((int)damage == hp_)
			reward = balance_;
		else
		{
			reward = static_cast<unsigned int>( std::round( double(damage) * double(balance_) / double(hp_) ) );
			reward = std::min(reward, balance_);
		}
	}

	hp_ -= damage;
	balance_ -= reward;

	miner->addBalance(reward);
}

void Resource::tick(unsigned long long tickCount)
{
	(void) tickCount;
}

std::unique_ptr<Object> & Resource::clone(Position newPos, Game * game) const
{
	int nextObjId = game->getNextObjectId();
	std::unique_ptr<Object> obj = std::make_unique<Resource>(nextObjId, newPos, balance_);
	obj->setHP(this->getHP());
	game->getObjects().push_back(std::move(obj));
	return game->getObjects().back();
}
