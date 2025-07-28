#include "Config.h"

std::string Config::serverConfigFilePath = "";

#include "JigsawWorldGenerator.h"
#include "DistancedResourceWorldGenerator.h"

static ServerConfig parseServerConfig()
{
	ServerConfig config;

	std::ifstream inFile(Config::getServerConfigFilePath());
	if (!inFile)
	{
		Logger::LogErr("Could not open server config file: " + Config::getServerConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);

	config.gameConfigFilePath = j.value("gameConfigFilePath", "config/game.json");
	config.replaysFolderPath = j.value("replaysFolderPath", "replays/");
	if (config.replaysFolderPath.back() == '/')
		config.replaysFolderPath.pop_back();
	config.dataFolderPath = j.value("dataFolderPath", "data/");
	if (config.dataFolderPath.back() == '/')
		config.dataFolderPath.pop_back();
	config.timeoutTicks = j.value("timeoutTicks", 3000);
	config.timeoutMs = j.value("timeoutMs", 3000);
	config.clientWaitTimeoutMs = j.value("clientWaitTimeoutMs", 500);
	config.enableTerminalVisualizer = j.value("enableTerminalVisualizer", false);

	return config;
}
static GameConfig parseGameConfig()
{
	GameConfig config;

	std::ifstream inFile(Config::server().gameConfigFilePath);
	if (!inFile)
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::server().gameConfigFilePath);
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);

	config.gridSize = j.value("gridSize", 25);
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
		config.corePositions.push_back({static_cast<int>(config.gridSize - 1), static_cast<int>(config.gridSize - 1)});
	}

	return config;
}

GameConfig &Config::game()
{
	static GameConfig configInstance = parseGameConfig();
	return configInstance;
}
ServerConfig &Config::server()
{
	static ServerConfig serverConfigInstance = parseServerConfig();
	return serverConfigInstance;
}

Position &Config::getCorePosition(unsigned int teamId)
{
	return game().corePositions[teamId];
}
UnitConfig &Config::getUnitConfig(unsigned int unit_type)
{
	return game().units[unit_type];
}

json Config::encodeConfig()
{
	std::ifstream inFile(Config::server().gameConfigFilePath);
	if (!inFile)
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::server().gameConfigFilePath);
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);
	return j;
}
