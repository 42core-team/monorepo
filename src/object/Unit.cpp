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
			move_cooldown_ = calcMoveCooldown();
	}
}

unsigned int Unit::calcMoveCooldown()
{
	unsigned int baseSpeed = Config::game().units[unit_type_].speed;
	unsigned int minSpeed = Config::game().units[unit_type_].minSpeed;

	float resourcePart = balance_ / (Config::game().resourceIncome / 4);
	if (resourcePart < 1)
		resourcePart = 1; // up to 1/4 resource balance does not slow down

	unsigned int speed = baseSpeed * resourcePart;

	if (speed > minSpeed)
		speed = minSpeed;

	return speed + 1;
}
