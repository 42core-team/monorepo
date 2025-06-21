#include "Core.h"

Core::Core(unsigned int id, unsigned int teamId, Position pos)
	: Object(id, pos, Config::instance().coreHp, ObjectType::Core), balance_(Config::instance().initialBalance), team_id_(teamId) {}

void Core::tick(unsigned long long tickCount)
{
	if (tickCount < Config::instance().idleIncomeTimeOut)
		setBalance(getBalance() + Config::instance().idleIncome);
}
