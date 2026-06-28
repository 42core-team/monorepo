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
	static const std::vector<std::string> adjectives = {
			"Ashen",	"Burned",	  "Irradiated", "Scorched",	 "Blasted",		 "Frozen",	  "Grim",	 "Wasteland",
			"Dustborn", "Feral",	  "Scarred",	"Savage",	 "Vengeful",	 "Raging",	  "Furious", "Wrathful",
			"Vicious",	"Relentless", "Unyielding", "Merciless", "Bloodstained", "Desperate", "Defiant", "Red",
			"Black",	"Crimson",	  "Golden",		"Silver",	 "Volatile"};
	static const std::vector<std::string> nouns = {"Goblin",
												   "Skeleton",
												   "Daemon",
												   "Ghoul",
												   "Husk",
												   "Mutant",
												   "Crawler",
												   "Beast",
												   "Vermin",
												   "Survivor",
												   "Scavenger",
												   "Raider",
												   "Wanderer",
												   "Heretic",
												   "Marauder",
												   "Lunatic",
												   "Castaway",
												   "Nomad",
												   "Rogue",
												   "Brute",
												   "Agent",
												   "Orphan",
												   "Outcast",
												   "Fugitive",
												   "Runner",
												   "Dieter Schwarz Bär",
												   "Theo-looking creature"};
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

	name_ = msg["name"].is_null() ? "" : msg["name"].get<std::string>();
}

json CreateAction::encodeJSON()
{
	json js;

	js["type"] = "create";
	js["name"] = name_.empty() ? nullptr : json(name_);
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
		ComponentConfig *component = Config::getComponentConfig(componentId);
		if (!component) return "invalid component \"" + componentId + "\"";

		componentCounts[componentId]++;
		unitCost += component->cost;
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
