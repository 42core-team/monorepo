#include "Config.h"

#include "JigsawWorldGenerator.h"
#include "DistancedResourceWorldGenerator.h"

namespace {
	std::unique_ptr<GameConfig> configInstance;
}

GameConfig parseConfig() {
	GameConfig config;

	std::ifstream inFile("config.json");
	if (!inFile) {
		std::cerr << "Error: Could not open config.json" << std::endl;
		exit(EXIT_FAILURE);
	}

	json j;
	inFile >> j;

	config.width            = j.value("width", 25);
	config.height           = j.value("height", 25);
	config.tickRate         = j.value("tickRate", 5);
	config.idleIncome       = j.value("idleIncome", 1);
	config.idleIncomeTimeOut= j.value("idleIncomeTimeOut", 600);
	config.resourceHp       = j.value("resourceHp", 50);
	config.resourceIncome   = j.value("resourceIncome", 200);
	config.moneyObjIncome   = j.value("moneyObjIncome", 100);
	config.coreHp           = j.value("coreHp", 350);
	config.initialBalance   = j.value("initialBalance", 200);
	config.wallHp           = j.value("wallHp", 100);
	config.wallBuildCost    = j.value("wallBuildCost", 20);

	std::string wgType = j.value("worldGenerator", "distanced_resources");
	if (wgType == "jigsaw")
		config.worldGenerator = std::make_unique<JigsawWorldGenerator>();
	else if (wgType == "distanced_resources")
		config.worldGenerator = std::make_unique<DistancedResourceWorldGenerator>();
	else
		config.worldGenerator = std::make_unique<JigsawWorldGenerator>();
	config.worldGeneratorConfig = j.value("worldGeneratorConfig", json());

	if (j.contains("units") && j["units"].is_array())
	{
		for (const auto &unitJson : j["units"])
		{
			UnitConfig unit;
			unit.name           = unitJson.value("name", "Unnamed");
			unit.cost           = unitJson.value("cost", 0);
			unit.hp             = unitJson.value("hp", 0);
			unit.speed          = unitJson.value("speed", 0);
			unit.minSpeed       = unitJson.value("minSpeed", 0);
			unit.damageCore     = unitJson.value("damageCore", 0);
			unit.damageUnit     = unitJson.value("damageUnit", 0);
			unit.damageResource = unitJson.value("damageResource", 0);
			unit.damageWall     = unitJson.value("damageWall", 0);
			int attackTypeInt   = unitJson.value("attackType", 0);
			unit.attackType     = static_cast<AttackType>(attackTypeInt);
			unit.attackReach    = unitJson.value("attackReach", 0);
			unit.canBuild       = unitJson.value("canBuild", false);

			config.units.push_back(unit);
		}
	}

	if (j.contains("corePositions") && j["corePositions"].is_array())
	{
		for (const auto &posJson : j["corePositions"])
		{
			Position pos;
			pos.x = posJson.value("x", 0);
			pos.y = posJson.value("y", 0);
			config.corePositions.push_back(pos);
		}
	}
	else
	{
		Logger::Log(LogLevel::ERROR, "No core positions found in config. Using default positions. Please fix this.");
		config.corePositions.push_back({ 0, 0 });
		config.corePositions.push_back({ static_cast<int>(config.width - 1), static_cast<int>(config.height - 1) });
	}

	return config;
}

GameConfig & Config::getInstance()
{
	if (!configInstance)
		configInstance = std::make_unique<GameConfig>(parseConfig());
	return *configInstance;
}

Position & Config::getCorePosition(unsigned int teamId)
{
	return getInstance().corePositions[teamId];
}
UnitConfig & Config::getUnitConfig(unsigned int typeId)
{
	return getInstance().units[typeId];
}

json Config::encodeConfig()
{
	const GameConfig& config = Config::getInstance();

	json configJson;

	configJson["width"] = config.width;
	configJson["height"] = config.height;
	configJson["tickRate"] = config.tickRate;

	configJson["idleIncome"] = config.idleIncome;
	configJson["idleIncomeTimeOut"] = config.idleIncomeTimeOut;

	configJson["resourceHp"] = config.resourceHp;
	configJson["resourceIncome"] = config.resourceIncome;
	configJson["moneyObjIncome"] = config.moneyObjIncome;

	configJson["coreHp"] = config.coreHp;
	configJson["initialBalance"] = config.initialBalance;

	configJson["wallHp"] = config.wallHp;
	configJson["wallBuildCost"] = config.wallBuildCost;

	configJson["units"] = json::array();
	for (auto& unit : config.units)
	{
		json u;

		u["name"] = unit.name;
		u["cost"] = unit.cost;
		u["hp"] = unit.hp;
		u["speed"] = unit.speed;
		u["minSpeed"] = unit.minSpeed;

		u["damageCore"] = unit.damageCore;
		u["damageUnit"] = unit.damageUnit;
		u["damageResource"] = unit.damageResource;
		u["damageWall"] = unit.damageWall;
		u["attackType"] = (int)unit.attackType;
		u["attackReach"] = unit.attackReach;

		u["canBuild"] = unit.canBuild;

		configJson["units"].push_back(u);
	}

	return configJson;
}
