#include "Unit.h"

Unit::Unit(unsigned int id, unsigned int teamId, unsigned int unit_type)
	: Object(id, Config::game().units[unit_type].hp, ObjectType::Unit), unit_type_(unit_type), team_id_(teamId), balance_(0)
{
	resetMoveCooldown();
}

void Unit::tick(unsigned long long tickCount)
{
	(void)tickCount;
	if (move_cooldown_ > 0)
	{
		move_cooldown_--;
		if (calcMoveCooldown() < move_cooldown_)
			move_cooldown_ = std::max(1u, calcMoveCooldown());
	}
}

unsigned int Unit::calcMoveCooldown()
{
	const auto& u = Config::game().units[unit_type_];
	unsigned int step = std::max(1u, u.balancePerCooldownStep);
	unsigned int steps = balance_ / step;

	unsigned int cd = u.baseMoveCooldown + steps;
	if (u.maxMoveCooldown > 0 && cd > u.maxMoveCooldown) cd = u.maxMoveCooldown;

	return std::max(1u, cd);
}
