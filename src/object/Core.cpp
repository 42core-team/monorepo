#include "Core.h"

#include "Game.h"

#include <cassert>

Core::Core(unsigned int id, unsigned int teamId, Position pos)
	: Object(id, pos, Config::getInstance().coreHp, ObjectType::Core), balance_(Config::getInstance().initialBalance), team_id_(teamId) {}

void Core::tick(unsigned long long tickCount)
{
	if (tickCount < Config::getInstance().idleIncomeTimeOut)
		setBalance(getBalance() + Config::getInstance().idleIncome);
}

std::unique_ptr<Object> & Core::clone(Position newPos, Game * game) const
{
	(void)newPos;
	(void)game;
	assert(false && "Core::clone() should never be called for Core objects");
	static std::unique_ptr<Object> dummy;
	return dummy;
}
