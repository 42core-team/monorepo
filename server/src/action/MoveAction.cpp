#include "MoveAction.h"

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
	if (!target_.isValid(Config::game().gridSize))
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

std::string MoveAction::execute(Core *core)
{
	if (!is_valid_) return "invalid input";

	Object *unitObj = Board::instance().getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;

	if (unit->getActionCooldown() > 0) return "unit is on cooldown";
	if (unit->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

	Object *obj = Board::instance().getObjectAtPos(target_);
	if (obj) return "invalid target position. should be empty";
	if (target_.distance(Board::instance().getObjectPositionById(unit->getId())) > 1) return "invalid move";

	Board::instance().moveObjectById(unit->getId(), target_);
	unit->resetActionCooldown();

	Stats::instance().inc(stat_keys::tiles_traveled);
	Stats::instance().inc(stat_keys::actions_executed);

	return "";
}
