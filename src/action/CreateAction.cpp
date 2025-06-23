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

bool CreateAction::execute(Core *core)
{
	if (!is_valid_)
		return false;

	Position closestEmptyPos = findFirstEmptyGridCell(Board::instance().getObjectPositionById(core->getId()));
	if (!closestEmptyPos.isValid(Config::instance().width, Config::instance().height))
		return false;

	if (unit_type_ >= Config::instance().units.size())
		return false;

	unsigned int unitCost = Config::getUnitConfig(unit_type_).cost;
	if (core->getBalance() < unitCost)
		return false;

	Board::instance().addObject<Unit>(Unit(Board::instance().getNextObjectId(), core->getTeamId(), unit_type_), closestEmptyPos);
	core->setBalance(core->getBalance() - unitCost);

	return true;
}
