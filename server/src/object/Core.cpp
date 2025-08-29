#include "Core.h"

Core::Core(unsigned int teamId)
	: Object(Config::game().coreHp, ObjectType::Core), balance_(Config::game().initialBalance), team_id_(teamId) {}

void Core::tick(unsigned long long tickCount)
{
	if (tickCount < Config::game().idleIncomeTimeOut)
		setBalance(getBalance() + Config::game().idleIncome);
}
