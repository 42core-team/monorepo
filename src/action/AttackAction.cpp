#include "AttackAction.h"

#include "Unit.h"

AttackAction::AttackAction(json msg) : Action(ActionType::MOVE)
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
	target_pos_.x = msg["target_pos_x"];
	target_pos_.y = msg["target_pos_y"];

	if (!target_pos_.isValid(Config::getInstance().width, Config::getInstance().height))
	{
		is_valid_ = false;
		return;
	}
}
json AttackAction::encodeJSON()
{
	json js;

	js["type"] = "move";
	js["unit_id"] = unit_id_;
	js["target_pos_x"] = target_pos_.x;
	js["target_pos_y"] = target_pos_.y;
	js["damage"] = damage_;

	return js;
}

bool AttackAction::attackObj(Object *obj, Unit * unit, Game *game) // returns object new hp, 1 if no object present
{
	if (!obj)
		return false;
	if (obj->getType() == ObjectType::Unit)
	{
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageUnit);
		damage_ = Config::getInstance().units[unit->getTypeId()].damageUnit;
	}
	else if (obj->getType() == ObjectType::Core)
	{
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageCore);
		damage_ = Config::getInstance().units[unit->getTypeId()].damageCore;
	}
	else if (obj->getType() == ObjectType::Resource)
	{
		((Resource *)obj)->getMined(unit);
		damage_ = Config::getInstance().units[unit->getTypeId()].damageResource;
	}
	else if (obj->getType() == ObjectType::Wall)
	{
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageWall);
		damage_ = Config::getInstance().units[unit->getTypeId()].damageWall;
	}
	else if (obj->getType() == ObjectType::Money)
	{
		unit->setBalance(unit->getBalance() + ((Money *)obj)->getBalance());
		game->removeObjectById(obj->getId());
		damage_ = 1;
	}

	return true;
}

bool AttackAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return false;

	Object * unitObj = game->getObject(getUnitId());

	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return false;
	Unit * unit = (Unit *)unitObj;
	if (unit->getNextMoveOpp() > 0)
		return false;
	if (unit->getTeamId() != core->getTeamId())
		return false;

	if (Config::getInstance().units[unit->getTypeId()].attackType == AttackType::DIRECT_HIT)
	{
		Object * obj = game->getObjectAtPos(target_pos_);
		if (!obj)
			return false;

		if (!attackObj(obj, unit, game))
			return false;
	}
	else if (Config::getInstance().units[unit->getTypeId()].attackType == AttackType::DROP_BOMB)
	{
		Object * obj = game->getObjectAtPos(target_pos_);
		if (obj)
			return false;

		game->getObjects().push_back(
			std::make_unique<Bomb>(game->getNextObjectId(), target_pos_));
	}
	else
	{
		Logger::Log(LogLevel::WARNING, "Unknown attack type for unit " + std::to_string(unit->getId()) + " of type " + std::to_string(unit->getTypeId()));
		return false;
	}

	return true;
}
