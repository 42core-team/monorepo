#include "MoveAction.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE)
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

static void attackObj(Object *obj, Unit * unit) // returns object new hp, 1 if no object present
{
	if (!obj)
		return;
	if (obj->getType() == ObjectType::Unit)
	{
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageUnit);
		if (obj->getHP() <= 0)
		{
			unit->addBalance(((Unit *)obj)->getBalance());
			((Unit *)obj)->setBalance(0);
		}
	}
	else if (obj->getType() == ObjectType::Core)
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageCore);
	else if (obj->getType() == ObjectType::Resource)
		((Resource *)obj)->getMined(unit);
	else if (obj->getType() == ObjectType::Wall)
		obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageWall);
}

void MoveAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return;

	Object * unitObj = game->getObject(getUnitId());

	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return;
	Unit * unit = (Unit *)unitObj;
	if (!unit->canTravel())
		return;
	if (unit->getTeamId() != core->getTeamId())
		return;

	Position unitPos = unit->getPosition();
	Position newPos = unitPos + getDirection();
	if (!newPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return;

	AttackType attackType = Config::getInstance().units[unit->getTypeId()].attackType;

	Object * obj = game->getObjectAtPos(newPos);
	if (!obj)
		unit->setPosition(newPos);

	if (attackType == AttackType::DIRECT_HIT)
	{
		attackObj(obj, unit);
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
			attackObj(shotObj, unit);
		}
	}
	else
	{
		std::cerr << "Unknown attack type" << std::endl;
	}

	unit->resetNextMoveOpp();
}
