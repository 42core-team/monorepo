#include "AttackAction.h"

#include "Unit.h"

AttackAction::AttackAction(json msg) : Action(ActionType::MOVE)
{
	decodeJSON(msg);
}

void AttackAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("dir"))
	{
		is_valid_ = false;
		return;
	}

	unit_id_ = msg["unit_id"];

	std::string dir = msg["dir"];
	if (dir == "u")
		dir_ = MovementDirection::UP;
	else if (dir == "d")
		dir_ = MovementDirection::DOWN;
	else if (dir == "l")
		dir_ = MovementDirection::LEFT;
	else if (dir == "r")
		dir_ = MovementDirection::RIGHT;
	else
		is_valid_ = false;
}
json AttackAction::encodeJSON()
{
	json js;

	js["type"] = "move";
	js["unit_id"] = unit_id_;
	switch (dir_)
	{
		case MovementDirection::UP:
			js["dir"] = "u";
			break;
		case MovementDirection::DOWN:
			js["dir"] = "d";
			break;
		case MovementDirection::LEFT:
			js["dir"] = "l";
			break;
		case MovementDirection::RIGHT:
			js["dir"] = "r";
			break;
	}
	js["target_id"] = target_id_;
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
	if (!unit->canTravel())
		return false;
	if (unit->getTeamId() != core->getTeamId())
		return false;

	Position unitPos = unit->getPosition();
	Position newPos = unitPos + getDirection();
	if (!newPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return false;

	Object * obj = game->getObjectAtPos(newPos);
	if (!obj)
		return false;

	if (!attackObj(obj, unit, game))
		return false;
	
	target_id_ = obj->getId();

	return true;
}
