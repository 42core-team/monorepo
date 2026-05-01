#include "CreateAction.h"

#include "Board.h"
#include "ComponentLogic.h"
#include "Config.h"
#include "Stats.h"
#include "Unit.h"

#include <unordered_map>

CreateAction::CreateAction(json msg) : Action(ActionType::CREATE)
{
	decodeJSON(msg);
}

void CreateAction::decodeJSON(json msg)
{
	components_.clear();

	for (const auto &componentIdJson : msg.at("components"))
	{
		components_.push_back(componentIdJson.get<std::string>());
	}
}

json CreateAction::encodeJSON()
{
	json js;

	js["type"] = "create";
	js["components"] = components_;

	return js;
}

std::string CreateAction::execute(Core *core)
{
	if (core->getSpawnCooldown() > 0)
		return "core spawn cooldown not done - wait " + std::to_string(core->getSpawnCooldown()) + " ticks";

	Position closestEmptyPos = findFirstEmptyGridCell(Board::instance().getObjectPositionById(core->getId()));
	if (!closestEmptyPos.isValid(Config::game().gridSize)) return "no valid position found - entire grid is filled up";

	if (components_.size() > Config::game().maxComponentsPerUnit)
	{
		return "too many components - has " + std::to_string(components_.size()) + ", max is " +
			   std::to_string(Config::game().maxComponentsPerUnit);
	}

	std::map<std::string, unsigned int> componentCounts;
	unsigned int unitCost = 0;

	for (const std::string &componentId : components_)
	{
		ComponentConfig &component = Config::getComponentConfig(componentId);

		componentCounts[componentId]++;

		if (componentCounts[componentId] > component.maxAddable)
		{
			return "too many copies of component \"" + componentId + "\" - has " +
				   std::to_string(componentCounts[componentId]) + ", max is " + std::to_string(component.maxAddable);
		}

		unitCost += component.cost;
	}

	if (core->getBalance() < unitCost)
		return "insufficient funds - has " + std::to_string(core->getBalance()) + ", needs " + std::to_string(unitCost);

	std::map<UnitProperty, int> properties = ComponentLogic::getUnitProperties(componentCounts);
	Board::instance().addObject<Unit>(Unit(core->getTeamId(), properties, components_), closestEmptyPos);
	core->setBalance(core->getBalance() - unitCost);

	Stats::instance().inc(stat_keys::units_spawned);
	Stats::instance().inc(stat_keys::actions_executed);

	core->setSpawnCooldown(properties.at(UnitProperty::POST_SPAWN_CORE_COOLDOWN));

	return "";
}
