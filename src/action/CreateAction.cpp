#include "CreateAction.h"

CreateAction::CreateAction(json msg) : Action(ActionType::CREATE)
{
	decodeJSON(msg);
}

void CreateAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_type"))
	{
		is_valid_ = false;
		return;
	}

	unit_type_ = msg["unit_type"];
}
json CreateAction::encodeJSON()
{
	json js;

	js["type"] = "create";
	js["unit_type"] = unit_type_;

	return js;
}

bool CreateAction::execute(Game *game, Core *core)
{
	if (!is_valid_)
		return false;

	Position closestEmptyPos = findFirstEmptyGridCell(game, core->getPosition());
	if (!closestEmptyPos.isValid(Config::getInstance().width, Config::getInstance().height))
		return false;

	if (unit_type_ >= Config::getInstance().units.size())
		return false;

	unsigned int unitCost = Config::getUnitConfig(unit_type_).cost;
	if (core->getBalance() < unitCost)
		return false;

	game->getObjects().push_back(std::make_unique<Unit>(game->getNextObjectId(), core->getTeamId(), closestEmptyPos, unit_type_));
	core->setBalance(core->getBalance() - unitCost);

	return true;
}
