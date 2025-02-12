#include "CreateAction.h"

CreateAction::CreateAction(json msg) : Action(ActionType::CREATE)
{
	if (!msg.contains("type_id"))
	{
		is_valid_ = false;
		return;
	}

	type_id_ = msg["type_id"];
}

void CreateAction::execute(Game *game, Core * core)
{
	Position closestEmptyPos = findFirstEmptyGridCell(game, core->getPosition());
	if (!closestEmptyPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return;

	unsigned int unitType = getUnitTypeId();
	if (unitType >= Config::getInstance().units.size())
		return;

	unsigned int unitCost = Config::getUnitConfig(unitType).cost;
	if (core->getBalance() < unitCost)
		return;

	game->getObjects().push_back(std::make_unique<Unit>(game->getNextObjectId(), core->getTeamId(), closestEmptyPos, unitType));
	core->setBalance(core->getBalance() - unitCost);
}
