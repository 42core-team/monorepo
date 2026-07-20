#include "Unit.h"

#include <algorithm>
#include <limits>

Unit::Unit(unsigned int teamId, std::map<UnitProperty, int> properties, std::vector<std::string> components,
		   std::string name)
	: Object(properties.at(UnitProperty::HP), ObjectType::Unit), properties_(properties), components_(components),
	  name_(std::move(name)), team_id_(teamId), balance_(0)
{
	resetActionCooldown();
}

void Unit::tick(unsigned long long tickCount)
{
	(void)tickCount;
}

void Unit::tickActionCooldown()
{
	const unsigned int gemAdjustedActionCooldown = calcActionCooldown();
	if (action_cooldown_ > 0)
	{
		action_cooldown_--;
		if (gemAdjustedActionCooldown < static_cast<unsigned int>(action_cooldown_))
			action_cooldown_ = gemAdjustedActionCooldown;
		return;
	}

	const unsigned int unitStandingCooldownLimit =
			std::min(gemAdjustedActionCooldown, Config::game().maxUnitStandingCooldown);
	action_cooldown_--;
	if (-action_cooldown_ > static_cast<long long>(unitStandingCooldownLimit))
		action_cooldown_ = unitStandingCooldownLimit;
}

unsigned int Unit::calcActionCooldown()
{
	unsigned int step = static_cast<unsigned int>(std::max(1, properties_.at(UnitProperty::GEMS_PER_COOLDOWN_STEP)));
	unsigned long long steps = balance_ / step;
	unsigned long long cd =
			static_cast<unsigned int>(std::max(0, properties_.at(UnitProperty::BASE_ACTION_COOLDOWN))) + steps;

	return static_cast<unsigned int>(
			std::clamp(cd, 1ull, static_cast<unsigned long long>(std::numeric_limits<unsigned int>::max())));
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
