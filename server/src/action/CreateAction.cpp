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

std::string CreateAction::execute(Core *core)
{
	if (!is_valid_)
		return "invalid input";

	Position closestEmptyPos = findFirstEmptyGridCell(Board::instance().getObjectPositionById(core->getId()));
	if (!closestEmptyPos.isValid(Config::game().gridSize))
		return "no valid position found";

	if (unit_type_ >= Config::game().units.size() || unit_type_ < 0)
		return "invalid unit type";

	unsigned int unitCost = Config::getUnitConfig(unit_type_).cost;
	if (core->getBalance() < unitCost)
		return "insufficient funds";

	Board::instance().addObject<Unit>(Unit(core->getTeamId(), unit_type_), closestEmptyPos);
	core->setBalance(core->getBalance() - unitCost);

	return "";
}
