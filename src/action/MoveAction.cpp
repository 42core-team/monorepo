#include "MoveAction.h"

#include "Unit.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE)
{
	decodeJSON(msg);
}

void MoveAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("x") || !msg.contains("y"))
	{
		is_valid_ = false;
		return;
	}

	unit_id_ = msg["unit_id"];
	target_ = Position(msg["x"], msg["y"]);
	if (!target_.isValid(Config::game().gridSize, Config::game().gridSize))
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
	js["x"] = target_.x;
	js["y"] = target_.y;

	return js;
}

bool MoveAction::execute(Core * core)
{
	if (!is_valid_)
		return false;

	Object * unitObj = Board::instance().getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return false;
	Unit * unit = (Unit *)unitObj;

	if (unit->getMoveCooldown() > 0)
		return false;
	if (unit->getTeamId() != core->getTeamId())
		return false;

	Object * obj = Board::instance().getObjectAtPos(target_);
	if (obj)
		return false;
	if (target_.distance(Board::instance().getObjectPositionById(unit->getId())) > 1)
		return false;

	Board::instance().moveObjectById(unit->getId(), target_);
	unit->resetMoveCooldown();

	return true;
}
