#include "BuildAction.h"

BuildAction::BuildAction(json msg) : Action(ActionType::BUILD)
{
	decodeJSON(msg);
}

void BuildAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("x") || !msg.contains("y"))
	{
		is_valid_ = false;
		return;
	}

	builder_id_ = msg["unit_id"];
	position_ = Position(msg["x"], msg["y"]);

	if (!position_.isValid(Config::game().gridSize, Config::game().gridSize))
		is_valid_ = false;
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
	(void)core;
	if (!is_valid_)
		return "invalid input";

	Object *builderObj = Board::instance().getObjectById(builder_id_);
	if (builderObj == nullptr || builderObj->getType() != ObjectType::Unit)
		return "invalid or non-existing unit";

	Unit *builder = dynamic_cast<Unit *>(builderObj);
	if (builder->getTeamId() != core->getTeamId())
		return "unit does not belong to your team";

	if (builder->getMoveCooldown() > 0)
		return "unit is on cooldown";

	BuildType buildType = Config::game().units[builder->getUnitType()].buildType;
	if (buildType == BuildType::NONE)
		return "unit unable to build";

	if (Board::instance().getObjectAtPos(position_) != nullptr)
		return "position occupied";

	if (position_.distance(Board::instance().getObjectPositionById(builder->getId())) > 1)
		return "invalid position. must be up, down, left or right of the unit";

	if (buildType == BuildType::WALL)
	{
		if (builder->getBalance() < Config::game().wallBuildCost)
			return "insufficient funds";
		builder->resetMoveCooldown();
		builder->setBalance(builder->getBalance() - Config::game().wallBuildCost);
		Board::instance().addObject<Wall>(Wall(Board::instance().getNextObjectId()), position_);
	}
	else if (buildType == BuildType::BOMB)
	{
		if (builder->getBalance() < Config::game().bombThrowCost)
			return "insufficient funds";
		builder->resetMoveCooldown();
		builder->setBalance(builder->getBalance() - Config::game().bombThrowCost);
		Board::instance().addObject<Bomb>(Bomb(Board::instance().getNextObjectId()), position_);
	}

	return "";
}
