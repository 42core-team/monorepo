#include "MoveAction.h"

#include "Unit.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE)
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

	return js;
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

	Object * obj = game->getObjectAtPos(newPos);
	if (obj)
		return false;

	unit->setPosition(newPos);
	unit->resetNextMoveOpp();
	return true;
}
