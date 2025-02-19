#include "CreateAction.h"

CreateAction::CreateAction(json msg) : Action(ActionType::CREATE)
{
	decodeJSON(msg);
}

void CreateAction::decodeJSON(json msg)
{
	if (!msg.contains("type_id"))
	{
		is_valid_ = false;
		return;
	}

	type_id_ = msg["type_id"];
}
json CreateAction::encodeJSON()
{
	json js;

	js["type"] = "create";
	js["type_id"] = type_id_;

	return js;
}

bool CreateAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return false;

	Position closestEmptyPos = findFirstEmptyGridCell(game, core->getPosition());
	if (!closestEmptyPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return false;

	unsigned int unitType = getUnitTypeId();
	if (unitType >= Config::getInstance().units.size())
		return false;

	unsigned int unitCost = Config::getUnitConfig(unitType).cost;
	if (core->getBalance() < unitCost)
		return false;

	game->getObjects().push_back(std::make_unique<Unit>(game->getNextObjectId(), core->getTeamId(), closestEmptyPos, unitType));
	core->setBalance(core->getBalance() - unitCost);

	return true;
}
