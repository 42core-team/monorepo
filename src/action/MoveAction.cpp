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

	Position newPos = unit->getPosition() + getDirection();
	if (!newPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return;

	Object * obj = game->getObjectAtPos(newPos);
	if (obj)
	{
		if (obj->getType() == ObjectType::Unit)
		{
			obj->setHP(obj->getHP() - Config::getInstance().units[unit->getTypeId()].damageUnit);
			if (obj->getHP() <= 0)
			{
				unit->setPosition(newPos);
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
	else
	{
		unit->setPosition(newPos);
	}
	unit->resetNextMoveOpp();
}
