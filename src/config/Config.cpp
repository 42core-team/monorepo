#include "Config.h"

std::string Config::configFilePath = "";

#include "JigsawWorldGenerator.h"
#include "DistancedResourceWorldGenerator.h"

GameConfig parseConfig()
{
	GameConfig config;

	std::ifstream inFile(Config::getConfigFilePath());
	if (!inFile)
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::getConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);

	config.width = j.value("width", 25);
	config.height = j.value("height", 25);
	config.tickRate = j.value("tickRate", 5);
	config.timeout = j.value("timeout", 3000);
	config.idleIncome = j.value("idleIncome", 1);
	config.idleIncomeTimeOut = j.value("idleIncomeTimeOut", 600);
	config.resourceHp = j.value("resourceHp", 50);
	config.resourceIncome = j.value("resourceIncome", 200);
	config.moneyObjIncome = j.value("moneyObjIncome", 100);
	config.coreHp = j.value("coreHp", 350);
	config.initialBalance = j.value("initialBalance", 200);
	config.wallHp = j.value("wallHp", 100);
	config.wallBuildCost = j.value("wallBuildCost", 20);
	config.bombHp = j.value("bombHp", 100);
	config.bombCountdown = j.value("bombCountdown", 25);
	config.bombThrowCost = j.value("bombThrowCost", 50);
	config.bombReach = j.value("bombReach", 3);
	config.bombDamage = j.value("bombDamage", 100);

	std::string wgType = j.value("worldGenerator", "distanced_resources");
	if (wgType == "jigsaw")
		config.worldGenerator = std::make_unique<JigsawWorldGenerator>();
	else if (wgType == "distanced_resources")
		config.worldGenerator = std::make_unique<DistancedResourceWorldGenerator>();
	else
	{
		Logger::Log(LogLevel::WARNING, "Unknown world generator type: \"" + wgType + "\". Using jigsaw as default.");
		config.worldGenerator = std::make_unique<JigsawWorldGenerator>();
	}
	config.worldGeneratorConfig = j.value("worldGeneratorConfig", json());

	if (j.contains("units") && j["units"].is_array())
	{
		for (const auto &unitJson : j["units"])
		{
			UnitConfig unit;
			unit.name = unitJson.value("name", "Unnamed");
			unit.cost = unitJson.value("cost", 0);
			unit.hp = unitJson.value("hp", 0);
			unit.speed = unitJson.value("speed", 0);
			unit.minSpeed = unitJson.value("minSpeed", 0);
			unit.damageCore = unitJson.value("damageCore", 0);
			unit.damageUnit = unitJson.value("damageUnit", 0);
			unit.damageResource = unitJson.value("damageResource", 0);
			unit.damageWall = unitJson.value("damageWall", 0);
			unit.damageBomb = unitJson.value("damageBomb", 0);
			int buildTypeInt = unitJson.value("buildType", 0);
			unit.buildType = static_cast<BuildType>(buildTypeInt);

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
		config.corePositions.push_back({0, 0});
		config.corePositions.push_back({static_cast<int>(config.width - 1), static_cast<int>(config.height - 1)});
	}

	return config;
}

GameConfig &Config::instance()
{
	static GameConfig configInstance = parseConfig();
	return configInstance;
}

Position &Config::getCorePosition(unsigned int teamId)
{
	return instance().corePositions[teamId];
}
UnitConfig &Config::getUnitConfig(unsigned int unit_type)
{
	return instance().units[unit_type];
}

json Config::encodeConfig()
{
	const GameConfig &config = Config::instance();

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

	configJson["bombHp"] = config.bombHp;
	configJson["bombCountdown"] = config.bombCountdown;
	configJson["bombThrowCost"] = config.bombThrowCost;
	configJson["bombReach"] = config.bombReach;
	configJson["bombDamage"] = config.bombDamage;

	configJson["units"] = json::array();
	for (auto &unit : config.units)
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
		u["damageBomb"] = unit.damageBomb;

		u["buildType"] = unit.buildType;

		configJson["units"].push_back(u);
	}

	configJson["corePositions"] = json::array();
	for (const auto &pos : config.corePositions)
	{
		json posJson;
		posJson["x"] = pos.x;
		posJson["y"] = pos.y;
		configJson["corePositions"].push_back(posJson);
	}

	return configJson;
}
