#include "BuildAction.h"

BuildAction::BuildAction(json msg) : Action(ActionType::BUILD)
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

void BuildAction::execute(Game *game, Core * core)
{
	(void) core;
	if (!is_valid_)
		return;

	Object *builderObj = game->getObject(builder_id_);
	if (builderObj == nullptr || builderObj->getType() != ObjectType::Unit)
		return;
	Unit *builder = dynamic_cast<Unit *>(builderObj);
	if (!Config::getInstance().units[builder->getTypeId()].canBuild)
		return;

	if (game->getObjectAtPos(position_) != nullptr)
		return;

	if (builder->getBalance() < Config::getInstance().wallBuildCost)
		return;
	builder->setBalance(builder->getBalance() - Config::getInstance().wallBuildCost);

	game->getObjects().push_back(std::make_unique<Wall>(game->getNextObjectId(), position_));
}
