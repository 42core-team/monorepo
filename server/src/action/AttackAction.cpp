#include "AttackAction.h"

#include "Unit.h"

#include <algorithm>
#include <cmath>

AttackAction::AttackAction(json msg) : Action(ActionType::ATTACK)
{
	decodeJSON(msg);
}

void AttackAction::decodeJSON(json msg)
{
	unit_id_ = msg["unit_id"];
	target_id_ = msg["target_id"];
}
json AttackAction::encodeJSON()
{
	json js;

	js["type"] = "attack";
	js["unit_id"] = unit_id_;
	js["target_id"] = target_id_;

	return js;
}

std::string AttackAction::execute(Core *core)
{
	Object *unitObj = Board::instance().getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;

	Object *targetObj = Board::instance().getObjectById(getTargetId());
	if (!targetObj) return "invalid or non-existing target";
	Position target_pos_ = Board::instance().getObjectPositionById(getTargetId());

	if (Board::instance().getObjectPositionById(unit->getId()).distance(target_pos_) > 1) return "unit is too far away";
	if (unit->getActionCooldown() > 0)
		return "unit is on action cooldown (action cooldown should be 0 or less to perform an action)";
	if (unit->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

	Object *obj = Board::instance().getObjectAtPos(target_pos_);
	if (!obj) return "no object at target position";

	unit->resetActionCooldown();

	// apply attack damage depending on object type
	unsigned int damage = 1;
	if (obj->getType() == ObjectType::Unit)
		damage = unit->getProperties().at(UnitProperty::DAMAGE_UNIT);
	else if (obj->getType() == ObjectType::Core)
		damage = unit->getProperties().at(UnitProperty::DAMAGE_CORE);
	else if (obj->getType() == ObjectType::Deposit)
		damage = unit->getProperties().at(UnitProperty::DAMAGE_OBJECT);
	else if (obj->getType() == ObjectType::Wall)
		damage = unit->getProperties().at(UnitProperty::DAMAGE_OBJECT);

	int damageReductionPercent = 0;
	if (obj->getType() == ObjectType::Unit)
	{
		Unit *targetUnit = (Unit *)obj;
		damageReductionPercent = targetUnit->getProperties().at(UnitProperty::DAMAGE_REDUCTION_PERCENT);
	}
	const unsigned int baseDamage = damage;
	const int armorPercent = std::clamp(damageReductionPercent, 0, 100);
	const unsigned int roundedDamage =
			static_cast<unsigned int>(std::round(static_cast<double>(baseDamage) * (100 - armorPercent) / 100.0));
	// Positive attacks always do at least 1 damage no matter the armor; exactly 0 damage stays 0. that way no amount of armor makes unkillable
	damage = baseDamage == 0 ? 0 : std::max(1u, roundedDamage);

	obj->damage(unit, damage);

	Stats::instance().inc(stat_keys::actions_executed);
	Stats::instance().inc(stat_keys::damage_total, damage);

	return "";
}
