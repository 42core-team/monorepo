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
	
	if (!position_.isValid(Config::getInstance().width, Config::getInstance().height))
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

bool BuildAction::execute(Game *game, Core * core)
{
	(void) core;
	if (!is_valid_)
		return false;

	Object *builderObj = game->getObject(builder_id_);
	if (builderObj == nullptr || builderObj->getType() != ObjectType::Unit)
		return false;
	Unit *builder = dynamic_cast<Unit *>(builderObj);
	if (!Config::getInstance().units[builder->getTypeId()].canBuild)
		return false;

	if (game->getObjectAtPos(position_) != nullptr)
		return false;

	if (position_.distance(builder->getPosition()) > 1)
		return false;

	if (builder->getBalance() < Config::getInstance().wallBuildCost)
		return false;
	builder->setBalance(builder->getBalance() - Config::getInstance().wallBuildCost);

	game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), position_));

	return true;
}
