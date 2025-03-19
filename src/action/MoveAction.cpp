#include "MoveAction.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE), attacked_(false)
{
	decodeJSON(msg);
}

void MoveAction::decodeJSON(json msg)
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
json MoveAction::encodeJSON()
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
	if (attacked_)
		js["attacked"] = attacked_;

	return js;
}

bool MoveAction::attackObj(Object *obj, Unit * unit, Game *game) // returns object new hp, 1 if no object present
{
	if (!obj)
		return false;
	if (obj->getType() == ObjectType::Unit)
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageUnit);
	else if (obj->getType() == ObjectType::Core)
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageCore);
	else if (obj->getType() == ObjectType::Resource)
		((Resource *)obj)->getMined(unit);
	else if (obj->getType() == ObjectType::Wall)
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageWall);
	else if (obj->getType() == ObjectType::Money)
	{
		unit->setBalance(unit->getBalance() + ((Money *)obj)->getBalance());
		game->removeObjectById(obj->getId());
		return true;
	}

	attacked_ = true;

	return true;
}
bool MoveAction::execute(Game *game, Core * core)
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

	AttackType attackType = Config::getInstance().units[unit->getTypeId()].attackType;

	Object * obj = game->getObjectAtPos(newPos);
	if (!obj)
	{
		unit->setPosition(newPos);
		unit->resetNextMoveOpp();
		return true;
	}

	if (attackType == AttackType::DIRECT_HIT)
	{
		if (!attackObj(obj, unit, game))
			return false;
	}
	else if (attackType == AttackType::DIRECTION_SHOT)
	{
		int deltaX = newPos.x - unitPos.x;
		int deltaY = newPos.y - unitPos.y;
		unsigned int attackReach = Config::getInstance().units[unit->getTypeId()].attackReach;

		for (int i = -((int)attackReach); i <= (int)attackReach; i++)
		{
			if (i == 0)
				continue;

			Position posI(unitPos.x + deltaX * i, unitPos.y + deltaY * i);
			if (!posI.isValid(Config::getInstance().width, Config::getInstance().height))
				continue;

			Object* shotObj = game->getObjectAtPos(posI);
			if (!attackObj(shotObj, unit, game))
				return false;
		}
	}
	else
	{
		std::cerr << "Unknown attack type" << std::endl;
	}

	unit->resetNextMoveOpp();

	return true;
}
