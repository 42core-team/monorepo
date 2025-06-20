#include "MoveAction.h"

#include "Unit.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE)
{
	decodeJSON(msg);
}

void MoveAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("targetX") || !msg.contains("targetY"))
	{
		is_valid_ = false;
		return;
	}

	unit_id_ = msg["unit_id"];
	target_ = Position(msg["targetX"], msg["targetY"]);
	if (!target_.isValid(Config::getInstance().width, Config::getInstance().height))
	{
		is_valid_ = false;
		return;
	}
}
json MoveAction::encodeJSON()
{
	json js;

	js["type"] = "move";
	js["unit_id"] = unit_id_;
	js["targetX"] = target_.x;
	js["targetY"] = target_.y;

	return js;
}

bool MoveAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return false;

	Object * unitObj = game->board_.getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return false;
	Unit * unit = (Unit *)unitObj;

	if (unit->getNextMoveOpp() > 0)
		return false;
	if (unit->getTeamId() != core->getTeamId())
		return false;

	Object * obj = game->board_.getObjectAtPos(target_);
	if (obj)
		return false;
	if (target_.distance(unit->getPosition()) > 1)
		return false;

	unit->setPosition(target_);
	unit->resetNextMoveOpp();

	return true;
}
