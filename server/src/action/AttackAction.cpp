#include "AttackAction.h"

#include "Unit.h"

AttackAction::AttackAction(json msg) : Action(ActionType::ATTACK)
{
	decodeJSON(msg);
}

void AttackAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("target_id"))
	{
		is_valid_ = false;
		return;
	}

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
	if (!is_valid_) return "invalid input";

	Object *unitObj = Board::instance().getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;

	Object *targetObj = Board::instance().getObjectById(getTargetId());
	if (!targetObj) return "invalid or non-existing target";
	Position target_pos_ = Board::instance().getObjectPositionById(getTargetId());

	if (Board::instance().getObjectPositionById(unit->getId()).distance(target_pos_) > 1) return "unit is too far away";
	if (unit->getActionCooldown() > 0) return "unit is on action cooldown (action cooldown should be 0)";
	if (unit->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

	Object *obj = Board::instance().getObjectAtPos(target_pos_);
	if (!obj) return "no object at target position";

	unit->resetActionCooldown();

	// apply attack damage depending on object type
	unsigned int damage = 1;
	if (obj->getType() == ObjectType::Unit)
		damage = Config::game().units[unit->getUnitType()].damageUnit;
	else if (obj->getType() == ObjectType::Core)
		damage = Config::game().units[unit->getUnitType()].damageCore;
	else if (obj->getType() == ObjectType::Deposit)
		damage = Config::game().units[unit->getUnitType()].damageDeposit;
	else if (obj->getType() == ObjectType::Wall)
		damage = Config::game().units[unit->getUnitType()].damageWall;
	else if (obj->getType() == ObjectType::Bomb)
		damage = Config::game().units[unit->getUnitType()].damageBomb;
	obj->damage(unit, damage);

	Stats::instance().inc(stat_keys::actions_executed);
	Stats::instance().inc(stat_keys::damage_total, damage);

	return "";
}
