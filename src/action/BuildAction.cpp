#include "BuildAction.h"

BuildAction::BuildAction(json msg) : Action(ActionType::BUILD)
{
	decodeJSON(msg);
}

void BuildAction::decodeJSON(json msg)
{
	if (!msg.contains("builder_id") || !msg.contains("x") || !msg.contains("y"))
	{
		is_valid_ = false;
		return;
	}

	builder_id_ = msg["builder_id"];
	position_ = Position(msg["x"], msg["y"]);

	if (!position_.isValid(Config::game().width, Config::game().height))
		is_valid_ = false;
}
json BuildAction::encodeJSON()
{
	json js;

	js["type"] = "build";
	js["builder_id"] = builder_id_;
	js["x"] = position_.x;
	js["y"] = position_.y;

	return js;
}

bool BuildAction::execute(Core *core)
{
	(void)core;
	if (!is_valid_)
		return false;

	Object *builderObj = Board::instance().getObjectById(builder_id_);
	if (builderObj == nullptr || builderObj->getType() != ObjectType::Unit)
		return false;
	Unit *builder = dynamic_cast<Unit *>(builderObj);
	BuildType buildType = Config::game().units[builder->getUnitType()].buildType;
	if (buildType == BuildType::NONE)
		return false;

	if (Board::instance().getObjectAtPos(position_) != nullptr)
		return false;

	if (position_.distance(Board::instance().getObjectPositionById(builder->getId())) > 1)
		return false;

	if (buildType == BuildType::WALL)
	{
		if (builder->getBalance() < Config::game().wallBuildCost)
			return false;
		builder->setBalance(builder->getBalance() - Config::game().wallBuildCost);
		Board::instance().addObject<Wall>(Wall(Board::instance().getNextObjectId()), position_);
	}
	else if (buildType == BuildType::BOMB)
	{
		if (builder->getBalance() < Config::game().bombThrowCost)
		return false;
		builder->setBalance(builder->getBalance() - Config::game().bombThrowCost);
		Board::instance().addObject<Bomb>(Bomb(Board::instance().getNextObjectId()), position_);
	}

	return true;
}
