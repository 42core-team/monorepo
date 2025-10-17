#include "BuildAction.h"

BuildAction::BuildAction(json msg) : Action(ActionType::BUILD)
{
	decodeJSON(msg);
}

void BuildAction::decodeJSON(json msg)
{
	builder_id_ = msg["unit_id"];
	position_ = Position(msg["x"], msg["y"]);
}
json BuildAction::encodeJSON()
{
	json js;

	js["type"] = "build";
	js["unit_id"] = builder_id_;
	js["x"] = position_.x;
	js["y"] = position_.y;

	return js;
}

std::string BuildAction::execute(Core *core)
{
	Object *builderObj = Board::instance().getObjectById(builder_id_);
	if (builderObj == nullptr || builderObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";

	Unit *builder = dynamic_cast<Unit *>(builderObj);
	if (builder->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

	if (builder->getActionCooldown() > 0) return "unit is on action cooldown (action cooldown should be 0)";

	BuildType buildType = Config::game().units[builder->getUnitType()].buildType;
	if (buildType == BuildType::NONE) return "unit unable to build";

	if (!position_.isValid(Config::game().gridSize)) return "target position out of bounds";
	if (Board::instance().getObjectAtPos(position_) != nullptr) return "position occupied";

	if (position_.distance(Board::instance().getObjectPositionById(builder->getId())) > 1)
		return "invalid position. must be up, down, left or right of the unit";

	if (buildType == BuildType::WALL)
	{
		if (builder->getBalance() < Config::game().wallBuildCost) return "insufficient funds of acting unit";
		builder->resetActionCooldown();
		builder->setBalance(builder->getBalance() - Config::game().wallBuildCost);
		Board::instance().addObject<Wall>(Wall(), position_);

		Stats::instance().inc(stat_keys::walls_spawned);
	}
	else if (buildType == BuildType::BOMB)
	{
		if (builder->getBalance() < Config::game().bombThrowCost) return "insufficient funds of acting unit";
		builder->resetActionCooldown();
		builder->setBalance(builder->getBalance() - Config::game().bombThrowCost);
		Board::instance().addObject<Bomb>(Bomb(), position_);

		Stats::instance().inc(stat_keys::bombs_spawned);
	}

	Stats::instance().inc(stat_keys::actions_executed);

	return "";
}
