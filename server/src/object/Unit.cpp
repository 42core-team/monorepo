#include "Unit.h"

Unit::Unit(unsigned int id, unsigned int teamId, unsigned int unit_type)
	: Object(id, Config::game().units[unit_type].hp, ObjectType::Unit), unit_type_(unit_type), team_id_(teamId), balance_(0)
{
	resetActionCooldown();
}

void Unit::tick(unsigned long long tickCount)
{
	(void)tickCount;
}

void Unit::tickActionCooldown()
{
	if (action_cooldown_ > 0)
	{
		action_cooldown_--;
		if (calcActionCooldown() < action_cooldown_)
			action_cooldown_ = std::max(1u, calcActionCooldown());
	}
}

unsigned int Unit::calcActionCooldown()
{
	const auto& u = Config::game().units[unit_type_];
	unsigned int step = std::max(1u, u.balancePerCooldownStep);
	unsigned int steps = balance_ / step;

	unsigned int cd = u.baseActionCooldown + steps;
	if (u.maxActionCooldown > 0 && cd > u.maxActionCooldown) cd = u.maxActionCooldown;

	return std::max(1u, cd);
}
