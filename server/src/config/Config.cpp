#include "Config.h"

#include <json-schema.hpp>
using nlohmann::json_schema::json_validator;

std::string Config::serverConfigFilePath = "";
std::string Config::gameConfigFilePath = "";
std::string Config::dataFolderPath = "";

#include "JigsawWorldGenerator.h"
#include "SparseWorldGenerator.h"
#include "HardcodedWorldGenerator.h"
#include "EmptyWorldGenerator.h"

static void validate_or_die(const json& instance, const std::string& schema_name) {
	std::string fullName = Config::getDataFolderPath() + "/config-schemas/" + schema_name;
	std::ifstream s(fullName);
	if (!s) { Logger::LogErr("Could not open schema: " + fullName); exit(EXIT_FAILURE); }
	json schema = json::parse(s);
	try { json_validator v; v.set_root_schema(schema); v.validate(instance); }
	catch (const std::exception& e) { Logger::Log(LogLevel::ERROR, std::string("Validation error: ") + e.what()); exit(EXIT_FAILURE); }
}

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
	validate_or_die(j, "server-config.schema.json");

	if (j.contains("replayFolderPaths") && j["replayFolderPaths"].is_array()) {
		config.replayFolderPaths.clear();
		for (const auto& path : j["replayFolderPaths"]) {
			if (path.is_string()) {
				std::string folderPath = path.get<std::string>();
				if (!folderPath.empty() && folderPath.back() == '/')
					folderPath.pop_back();
				config.replayFolderPaths.push_back(folderPath);
			}
		}
	} else {
		config.replayFolderPaths = {"replays"};
	}
	config.timeoutTicks = j.value("timeoutTicks", 3000);
	config.timeoutMs = j.value("timeoutMs", 3000);
	config.clientWaitTimeoutMs = j.value("clientWaitTimeoutMs", 500);
	config.clientConnectTimeoutMs = j.value("clientConnectTimeoutMs", 30000);
	config.enableTerminalVisualizer = j.value("enableTerminalVisualizer", false);

	return config;
}
static GameConfig parseGameConfig()
{
	GameConfig config;

	std::ifstream inFile(Config::getGameConfigFilePath());
	if (!inFile)
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::getGameConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);
	validate_or_die(j, "game-config.schema.json");

	config.gridSize = j.value("gridSize", 25);
	if (j.contains("seed") && j["seed"].is_number_unsigned())
		config.seed = std::clamp(j["seed"].get<uint64_t>(), uint64_t{1}, uint64_t{UINT64_MAX});
	else
	{
		Logger::LogErr("Incorrect seed format, please supply proper unsigned integer as seed.");
		config.seed = 1;
	}
	config.idleIncome = j.value("idleIncome", 1);
	config.idleIncomeTimeOut = j.value("idleIncomeTimeOut", 600);
	config.depositHp = j.value("depositHp", 50);
	config.depositIncome = j.value("depositIncome", 200);
	config.gemPileIncome = j.value("gemPileIncome", 100);
	config.coreHp = j.value("coreHp", 350);
	config.initialBalance = j.value("initialBalance", 200);
	config.wallHp = j.value("wallHp", 100);
	config.wallBuildCost = j.value("wallBuildCost", 20);
	config.bombHp = j.value("bombHp", 100);
	config.bombCountdown = j.value("bombCountdown", 25);
	config.bombThrowCost = j.value("bombThrowCost", 50);
	config.bombReach = j.value("bombReach", 3);
	config.bombDamageCore = j.value("bombDamageCore", 50);
	config.bombDamageUnit = j.value("bombDamageUnit", 30);
	config.bombDamageDeposit = j.value("bombDamageDeposit", 40);

	std::string wgType = j.value("worldGenerator", "jigsaw");
	if (wgType == "jigsaw")
		config.worldGenerator = std::make_unique<JigsawWorldGenerator>();
	else if (wgType == "sparse")
		config.worldGenerator = std::make_unique<SparseWorldGenerator>();
	else if (wgType == "hardcoded")
		config.worldGenerator = std::make_unique<HardcodedWorldGenerator>();
	else if (wgType == "empty")
		config.worldGenerator = std::make_unique<EmptyWorldGenerator>();
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
			unit.baseActionCooldown = unitJson.value("baseActionCooldown", 0);
			unit.maxActionCooldown = unitJson.value("maxActionCooldown", 0);
			unit.balancePerCooldownStep = std::max(1u, unitJson.value("balancePerCooldownStep", 1u));
			unit.damageCore = unitJson.value("damageCore", 0);
			unit.damageUnit = unitJson.value("damageUnit", 0);
			unit.damageDeposit = unitJson.value("damageDeposit", 0);
			unit.damageWall = unitJson.value("damageWall", 0);
			int buildTypeInt = unitJson.value("buildType", 0);
			unit.buildType = static_cast<BuildType>(buildTypeInt);

			if (unit.baseActionCooldown > unit.maxActionCooldown)
			{
				Logger::LogErr("baseActionCooldown > maxActionCooldown for unit: " + unit.name);
				exit(EXIT_FAILURE);
			}

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

			if (!pos.isValid(config.gridSize))
			{
				Logger::LogErr("Invalid core position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ").");
				exit(EXIT_FAILURE);
			}

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
	std::ifstream inFile(Config::getGameConfigFilePath());
	if (!inFile)
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::getGameConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(inFile);
	return j;
}
