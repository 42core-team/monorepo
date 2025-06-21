#include "Unit.h"

Unit::Unit(unsigned int id, unsigned int teamId, unsigned int unit_type)
	: Object(id, Config::instance().units[unit_type].hp, ObjectType::Unit), unit_type_(unit_type), team_id_(teamId), balance_(0)
{
	resetNextMoveOpp();
}

void Unit::tick(unsigned long long tickCount)
{
	(void)tickCount;
	if (next_move_opp_ > 0)
		next_move_opp_--;
}

unsigned int Unit::calcNextMovementOpp()
{
	unsigned int baseSpeed = Config::instance().units[unit_type_].speed;
	unsigned int minSpeed = Config::instance().units[unit_type_].minSpeed;

	float resourcePart = balance_ / (Config::instance().resourceIncome / 4);
	if (resourcePart < 1)
		resourcePart = 1; // up to 1/4 resource balance does not slow down

	unsigned int speed = baseSpeed * resourcePart;

	if (speed > minSpeed)
		speed = minSpeed;

	return speed;
}
