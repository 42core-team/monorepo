#include "AttackAction.h"

#include "Unit.h"

AttackAction::AttackAction(json msg) : Action(ActionType::ATTACK)
{
	decodeJSON(msg);
}

void AttackAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("x") || !msg.contains("y"))
	{
		is_valid_ = false;
		return;
	}

	unit_id_ = msg["unit_id"];
	target_pos_.x = msg["x"];
	target_pos_.y = msg["y"];

	if (!target_pos_.isValid(Config::game().gridSize, Config::game().gridSize))
	{
		is_valid_ = false;
		return;
	}
}
json AttackAction::encodeJSON()
{
	json js;

	js["type"] = "attack";
	js["unit_id"] = unit_id_;
	js["x"] = target_pos_.x;
	js["y"] = target_pos_.y;

	return js;
}

std::string AttackAction::attackObj(Object *obj, Unit *unit) // returns object new hp, 1 if no object present
{
	if (!obj)
		return "no object at target position";
	unit->resetMoveCooldown();
	if (obj->getType() == ObjectType::Unit)
	{
		obj->setHP(obj->getHP() - Config::game().units[unit->getUnitType()].damageUnit);
	}
	else if (obj->getType() == ObjectType::Core)
	{
		obj->setHP(obj->getHP() - Config::game().units[unit->getUnitType()].damageCore);
	}
	else if (obj->getType() == ObjectType::Resource)
	{
		obj->setHP(obj->getHP() - Config::game().units[unit->getUnitType()].damageResource);
		if (obj->getHP() <= 0)
		{
			unsigned int balance = ((Resource *)obj)->getBalance();
			Board::instance().removeObjectById(obj->getId());
			Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId(), balance), target_pos_, true);
		}
	}
	else if (obj->getType() == ObjectType::Wall)
	{
		obj->setHP(obj->getHP() - Config::game().units[unit->getUnitType()].damageWall);
	}
	else if (obj->getType() == ObjectType::Bomb)
	{
		Bomb *bomb = (Bomb *)obj;
		bomb->explode();
	}

	return "";
}

std::string AttackAction::execute(Core *core)
{
	if (!is_valid_)
		return "invalid input";

	Object *unitObj = Board::instance().getObjectById(getUnitId());

	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;
	if (Board::instance().getObjectPositionById(unit->getId()).distance(target_pos_) > 1)
		return "unit is too far away";
	if (unit->getMoveCooldown() > 0)
		return "unit is on cooldown";
	if (unit->getTeamId() != core->getTeamId())
		return "unit does not belong to your team";

	Object *obj = Board::instance().getObjectAtPos(target_pos_);
	if (!obj)
		return "no object at target position";

	return attackObj(obj, unit);
}
