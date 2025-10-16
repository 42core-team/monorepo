#include "Config.h"

#include <json-schema.hpp>
using nlohmann::json_schema::json_validator;

std::string Config::serverConfigFilePath = "";
std::string Config::gameConfigFilePath = "";
std::string Config::dataFolderPath = "";

#include "EmptyWorldGenerator.h"
#include "HardcodedWorldGenerator.h"
#include "JigsawWorldGenerator.h"
#include "SparseWorldGenerator.h"

#define XXH_INLINE_ALL
#include "Utils.h"
#include "xxhash.h"

json Config::load_json_schema(const std::string &schema_name)
{
	std::string fullName = Config::getDataFolderPath() + "/json-schemas/" + schema_name;
	std::ifstream s(fullName);
	if (!s)
	{
		Logger::LogErr("Could not open schema: " + fullName);
		exit(EXIT_FAILURE);
	}
	return json::parse(s);
}

static void validate_or_die(const json &instance, const std::string &schema_name)
{
	json schema = Config::load_json_schema(schema_name);
	try
	{
		json_validator v;
		v.set_root_schema(schema);
		v.validate(instance);
	}
	catch (const std::exception &e)
	{
		Logger::Log(LogLevel::ERROR, std::string("Validation error: ") + e.what());
		exit(EXIT_FAILURE);
	}
}

static std::string read_file_strip_json_comments(const std::string &path)
{
	std::ifstream in(path, std::ios::in | std::ios::binary);
	if (!in) return {};

	std::ostringstream raw;
	raw << in.rdbuf();
	std::string s = raw.str();
	std::string out;
	out.reserve(s.size());

	bool in_string = false;		// inside JSON string literal?
	bool escaped = false;		// previous char was a backslash inside a string
	bool in_sl_comment = false; // inside // ... \n
	bool in_ml_comment = false; // inside /* ... */
	const size_t n = s.size();

	for (size_t i = 0; i < n; ++i)
	{
		const char c = s[i];
		const char next = (i + 1 < n) ? s[i + 1] : '\0';

		if (in_sl_comment)
		{
			// consume until newline, but keep the newline
			if (c == '\n')
			{
				in_sl_comment = false;
				out.push_back('\n');
			}
			// else drop char
			continue;
		}

		if (in_ml_comment)
		{
			// consume until closing */
			if (c == '*' && next == '/')
			{
				in_ml_comment = false;
				++i; // skip '/'
			}
			else if (c == '\n')
			{
				// preserve newlines to keep line numbers stable
				out.push_back('\n');
			}
			continue;
		}

		// not in any comment
		if (!in_string)
		{
			// start of // comment?
			if (c == '/' && next == '/')
			{
				in_sl_comment = true;
				++i; // skip second '/'
				continue;
			}
			// start of /* comment?
			if (c == '/' && next == '*')
			{
				in_ml_comment = true;
				++i; // skip '*'
				continue;
			}
			// entering a string?
			if (c == '\"')
			{
				in_string = true;
				escaped = false;
				out.push_back(c);
				continue;
			}
			// normal char
			out.push_back(c);
		}
		else
		{
			// inside string literal
			out.push_back(c);
			if (escaped)
			{
				escaped = false;
			}
			else
			{
				if (c == '\\')
					escaped = true;
				else if (c == '\"')
					in_string = false;
			}
		}
	}

	return out;
}

static ServerConfig parseServerConfig()
{
	ServerConfig config;

	const std::string cleaned = read_file_strip_json_comments(Config::getServerConfigFilePath());
	if (cleaned.empty())
	{
		Logger::LogErr("Could not open server config file: " + Config::getServerConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(cleaned);
	validate_or_die(j, "server-config.schema.json");

	if (j.contains("replayFolderPaths") && j["replayFolderPaths"].is_array())
	{
		config.replayFolderPaths.clear();
		for (const auto &path : j["replayFolderPaths"])
		{
			if (path.is_string())
			{
				std::string folderPath = path.get<std::string>();
				if (!folderPath.empty() && folderPath.back() == '/') folderPath.pop_back();
				config.replayFolderPaths.push_back(folderPath);
			}
		}
	}
	else
	{
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

	const std::string cleaned = read_file_strip_json_comments(Config::getGameConfigFilePath());
	if (cleaned.empty())
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::getGameConfigFilePath());
		exit(EXIT_FAILURE);
	}

	json j = json::parse(cleaned);
	validate_or_die(j, "game-config.schema.json");

	config.gridSize = j.value("gridSize", 25);

	config.seedString = j.value("seed", "");
	if (config.seedString.empty())
	{
		config.seedString = random_base32_seed();
		config.usedRandomSeed = true;
	}
	config.seed = XXH64(config.seedString.data(), config.seedString.size(), 0);

	config.idleIncome = j.value("idleIncome", 1);
	config.idleIncomeTimeOut = j.value("idleIncomeTimeOut", 600);
	config.depositHp = j.value("depositHp", 50);
	config.depositIncome = j.value("depositIncome", 200);
	config.gemPileIncome = j.value("gemPileIncome", 100);
	config.coreHp = j.value("coreHp", 350);
	config.coreSpawnCooldown = j.value("coreSpawnCooldown", 20);
	config.initialBalance = j.value("initialBalance", 200);
	config.wallHp = j.value("wallHp", 100);
	config.wallBuildCost = j.value("wallBuildCost", 20);
	config.bombHp = j.value("bombHp", 25);
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
			unit.damageBomb = unitJson.value("damageBomb", 0);

			std::string buildTypeJson = unitJson.value("buildType", "none");
			if (buildTypeJson == "none")
				unit.buildType = BuildType::NONE;
			else if (buildTypeJson == "wall")
				unit.buildType = BuildType::WALL;
			else if (buildTypeJson == "bomb")
				unit.buildType = BuildType::BOMB;

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
				Logger::LogErr("Invalid core position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) +
							   ").");
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
	const std::string cleaned = read_file_strip_json_comments(Config::getGameConfigFilePath());
	if (cleaned.empty())
	{
		Logger::Log(LogLevel::ERROR, "Could not open config file: " + Config::getGameConfigFilePath());
		exit(EXIT_FAILURE);
	}
	return json::parse(cleaned);
}
