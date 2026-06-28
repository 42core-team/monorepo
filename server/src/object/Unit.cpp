#include "Unit.h"

Unit::Unit(unsigned int teamId, std::map<UnitProperty, int> properties, std::vector<std::string> components)
	: Object(properties.at(UnitProperty::HP), ObjectType::Unit), properties_(properties), components_(components),
	  team_id_(teamId), balance_(0)
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
		if (calcActionCooldown() < action_cooldown_) action_cooldown_ = std::max(1u, calcActionCooldown());
	}
}

unsigned int Unit::calcActionCooldown()
{
	unsigned int step = std::max(1, properties_.at(UnitProperty::BALANCE_PER_COOLDOWN_STEP));
	unsigned int steps = balance_ / step;
	unsigned int cd = std::max(0, properties_.at(UnitProperty::BASE_ACTION_COOLDOWN)) + steps;

	return std::max(1u, cd);
}

void Unit::damage(Object *attacker, unsigned int damage)
{
	this->setHP(this->getHP() - damage);

	Stats::instance().inc(stat_keys::damage_units, damage);
	if (attacker->getType() != ObjectType::Unit) return;
	if (((Unit *)attacker)->getTeamId() != team_id_)
		Stats::instance().inc(stat_keys::damage_opponent, damage);
	else
		Stats::instance().inc(stat_keys::damage_self, damage);
}
