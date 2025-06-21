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

	if (!position_.isValid(Config::instance().width, Config::instance().height))
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
	if (!Config::instance().units[builder->getUnitType()].canBuild)
		return false;

	if (Board::instance().getObjectAtPos(position_) != nullptr)
		return false;

	if (position_.distance(builder->getPosition()) > 1)
		return false;

	if (builder->getBalance() < Config::instance().wallBuildCost)
		return false;
	builder->setBalance(builder->getBalance() - Config::instance().wallBuildCost);

	Board::instance().addObject<Wall>(Wall(Board::instance().getNextObjectId(), position_));

	return true;
}
