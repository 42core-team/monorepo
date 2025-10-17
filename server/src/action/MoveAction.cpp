#include "MoveAction.h"

MoveAction::MoveAction(json msg) : Action(ActionType::MOVE)
{
	decodeJSON(msg);
}

void MoveAction::decodeJSON(json msg)
{
	unit_id_ = msg["unit_id"];
	target_ = Position(msg["x"], msg["y"]);
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
	Object *unitObj = Board::instance().getObjectById(getUnitId());
	if (!unitObj || unitObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;

	if (unit->getActionCooldown() > 0)
		return "unit is on action cooldown (action cooldown should be 0) or has already moved this tick";
	if (unit->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

	if (!target_.isValid(Config::game().gridSize)) return "target position out of bounds";
	if (Board::instance().getObjectAtPos(target_)) return "invalid target position. should be empty";
	if (target_.distance(Board::instance().getObjectPositionById(unit->getId())) > 1)
		return "targeted position too far away";

	Board::instance().moveObjectById(unit->getId(), target_);
	unit->resetActionCooldown();

	Stats::instance().inc(stat_keys::tiles_traveled);
	Stats::instance().inc(stat_keys::actions_executed);

	return "";
}
