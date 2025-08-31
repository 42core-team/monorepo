#include "Core.h"

Core::Core(unsigned int teamId)
	: Object(Config::game().coreHp, ObjectType::Core), balance_(Config::game().initialBalance), team_id_(teamId) {}

void Core::tick(unsigned long long tickCount)
{
	if (tickCount < Config::game().idleIncomeTimeOut)
		setBalance(getBalance() + Config::game().idleIncome);
}

void Core::damage(Object *attacker, unsigned int damage)
{
	this->setHP(this->getHP() - damage);

	Stats::instance().inc(stat_keys::damage_cores, damage);
	if (((Core *)this)->getTeamId() != ((Core *)attacker)->getTeamId())
		Stats::instance().inc(stat_keys::damage_opponent, damage);
	else
		Stats::instance().inc(stat_keys::damage_self, damage);
}
