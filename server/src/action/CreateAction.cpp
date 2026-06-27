#include "CreateAction.h"

#include "Board.h"
#include "ComponentLogic.h"
#include "Config.h"
#include "Stats.h"
#include "Unit.h"

#include <random>
#include <unordered_map>

static std::string generateUnitName()
{
	static const std::vector<std::string> adjectives = {"Iron",	  "Shadow",	 "Swift",  "Bold",	"Stone", "Dark",
														"Silver", "Grim",	 "Frost",  "Ember", "Ashen", "Dread",
														"Wild",	  "Crimson", "Silent", "Hollow"};
	static const std::vector<std::string> nouns = {"Wolf",	 "Hawk",   "Blade",	   "Lance",	  "Shield", "Claw",
												   "Fang",	 "Arrow",  "Warden",   "Striker", "Rover",	"Specter",
												   "Reaper", "Hunter", "Sentinel", "Wraith"};
	static std::mt19937 rng(std::random_device{}());
	std::uniform_int_distribution<size_t> adjDist(0, adjectives.size() - 1);
	std::uniform_int_distribution<size_t> nounDist(0, nouns.size() - 1);
	return adjectives[adjDist(rng)] + " " + nouns[nounDist(rng)];
}

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

	name_ = msg.contains("name") ? msg["name"].get<std::string>() : "";
}

json CreateAction::encodeJSON()
{
	json js;

	js["type"] = "create";
	if (!name_.empty()) js["name"] = name_;
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
		try
		{
			const ComponentConfig *component = Config::getComponentConfig(componentId);
			if (!component) continue;

			componentCounts[componentId]++;
			unitCost += component.cost;
		}
		catch (const std::exception &)
		{
			return "invalid component \"" + componentId + "\"";
		}
	}

	if (core->getBalance() < unitCost)
		return "insufficient funds - has " + std::to_string(core->getBalance()) + ", needs " + std::to_string(unitCost);

	std::map<UnitProperty, int> properties = ComponentLogic::getUnitProperties(componentCounts);

	const std::string invalidConditionMessage = ComponentLogic::getInvalidConditionMessage(properties, componentCounts);
	if (!invalidConditionMessage.empty()) return invalidConditionMessage;

	std::string name = name_.empty() ? generateUnitName() : name_;
	Board::instance().addObject<Unit>(Unit(core->getTeamId(), properties, components_, name), closestEmptyPos);
	core->setBalance(core->getBalance() - unitCost);

	Stats::instance().inc(stat_keys::units_spawned);
	Stats::instance().inc(stat_keys::actions_executed);

	core->setSpawnCooldown(properties.at(UnitProperty::POST_SPAWN_CORE_COOLDOWN));

	return "";
}
