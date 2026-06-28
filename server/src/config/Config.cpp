#include "Config.h"

#include <json-schema.hpp>
#include <unordered_map>
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

#include <set>
#include <stdexcept>

json Config::load_json_schema(const std::string &schema_name)
{
	std::string fullName = Config::getDataFolderPath() + "/json-schemas/" + schema_name;
	std::ifstream s(fullName);
	if (!s)
	{
		Logger::LogErr("Could not open schema: " + fullName);
		exit(EXIT_FAILURE);
	}
	try
	{
		return json::parse(s);
	}
	catch (const std::exception &e)
	{
		Logger::LogErr("Failed to parse schema " + schema_name + ": " + std::string(e.what()));
		exit(EXIT_FAILURE);
	}
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

UnitProperty Config::stringToUnitProperty(std::string_view name)
{
	for (const auto &[entryName, property] : UNIT_PROPERTY_ENTRIES)
		if (entryName == name) return property;
	throw std::runtime_error("Unknown unit property name: \"" + std::string(name) + "\".");
}
std::string_view Config::unitPropertyToString(UnitProperty property)
{
	for (const auto &[name, entryProperty] : UNIT_PROPERTY_ENTRIES)
		if (entryProperty == property) return name;
	throw std::runtime_error("Unknown UnitProperty value.");
}

// json schema isnt recursive, so we can validate the invalid condition expressions with a simple recursive function instead of using json-schema
static void requireExprKeys(const json &expr, const std::set<std::string> &keys)
{
	for (auto it = expr.begin(); it != expr.end(); ++it)
		if (!keys.count(it.key())) throw std::runtime_error("Unexpected invalid condition key: \"" + it.key() + "\".");
	for (const auto &key : keys)
		if (!expr.contains(key)) throw std::runtime_error("Missing invalid condition key: \"" + key + "\".");
}
static void validateInvalidConditionExpr(const json &expr)
{
	if (!expr.is_object()) throw std::runtime_error("Invalid condition expression must be an object.");
	const std::string type = expr.at("type").get<std::string>();

	if (type == "const")
	{
		requireExprKeys(expr, {"type", "value"});
		const int64_t value = expr.at("value").get<int64_t>();
		if (value < -10000000 || value > 10000000) throw std::runtime_error("Invalid condition const is out of range.");
		return;
	}
	if (type == "property")
	{
		requireExprKeys(expr, {"type", "property"});
		Config::stringToUnitProperty(expr.at("property").get<std::string>());
		return;
	}
	if (type == "component_count")
	{
		requireExprKeys(expr, {"type", "component"});
		if (expr.at("component").get<std::string>().empty())
			throw std::runtime_error("Invalid condition component must not be empty.");
		return;
	}
	if (type == "not")
	{
		requireExprKeys(expr, {"type", "1"});
		validateInvalidConditionExpr(expr.at("1"));
		return;
	}
	if (type == "ternary")
	{
		requireExprKeys(expr, {"type", "if", "then", "else"});
		validateInvalidConditionExpr(expr.at("if"));
		validateInvalidConditionExpr(expr.at("then"));
		validateInvalidConditionExpr(expr.at("else"));
		return;
	}

	static const std::set<std::string> binaryTypes = {"sum",
													  "subtract",
													  "multiply",
													  "divide",
													  "min",
													  "max",
													  "less_than",
													  "greater_than",
													  "less_than_or_equal",
													  "greater_than_or_equal",
													  "equal"};
	if (!binaryTypes.count(type)) throw std::runtime_error("Unknown invalid condition type: \"" + type + "\".");
	requireExprKeys(expr, {"type", "1", "2"});
	validateInvalidConditionExpr(expr.at("1"));
	validateInvalidConditionExpr(expr.at("2"));
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
	validate_or_die(j, "configs/server-config.schema.json");

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
	config.timeoutTicks = j.at("timeoutTicks").get<unsigned int>();
	config.timeoutMs = j.at("timeoutMs").get<unsigned int>();
	config.clientWaitTimeoutMs = j.at("clientWaitTimeoutMs").get<unsigned int>();
	config.clientConnectTimeoutMs = j.at("clientConnectTimeoutMs").get<unsigned int>();
	config.clientPacketsMaxSizeKb = j.at("clientPacketsMaxSizeKb").get<unsigned int>();

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
	validate_or_die(j, "configs/game-config.schema.json");

	config.gridSize = j.at("gridSize").get<unsigned int>();

	config.seedString = j.value("seed", "");
	if (config.seedString.empty())
	{
		config.seedString = random_base32_seed();
		config.usedRandomSeed = true;
	}
	config.seed = XXH64(config.seedString.data(), config.seedString.size(), 0);

	config.idleIncome = j.at("idleIncome").get<unsigned int>();
	config.idleIncomeTimeOut = j.at("idleIncomeTimeOut").get<unsigned int>();

	config.depositHp = j.at("depositHp").get<unsigned int>();
	config.depositIncome = j.at("depositIncome").get<unsigned int>();
	config.gemPileIncome = j.at("gemPileIncome").get<unsigned int>();

	config.coreHp = j.at("coreHp").get<unsigned int>();
	config.initialBalance = j.at("initialBalance").get<unsigned int>();

	config.wallHp = j.at("wallHp").get<unsigned int>();

	std::string wgType = j.at("worldGenerator").get<std::string>();
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
		throw std::runtime_error("Unknown world generator type: \"" + wgType + "\".");
	}
	config.worldGeneratorConfig = j.at("worldGeneratorConfig").get<json>();

	json components = j.at("components").get<json>();

	config.maxComponentsPerUnit = components.at("maxComponentsPerUnit").get<unsigned int>();

	const json &defaults = components.at("unitDefaultProperties");
	for (const auto &[name, valueJson] : defaults.items())
	{
		UnitProperty propType = Config::stringToUnitProperty(name);
		int value = valueJson.get<int>();
		config.defaultUnitProperties[propType] = value;
	}

	std::set<std::string> seenComponentIds;
	for (const auto &componentJson : components.at("components"))
	{
		ComponentConfig comp;
		comp.id = componentJson.at("id").get<std::string>();
		if (!seenComponentIds.insert(comp.id).second)
			throw std::runtime_error("Duplicate component id: \"" + comp.id + "\".");
		std::set<UnitProperty> seenProperties;
		for (const auto &propJson : componentJson.at("properties"))
		{
			UnitProperty propType = Config::stringToUnitProperty(propJson.at("name").get<std::string>());
			int modification = propJson.at("modification").get<int>();
			if (!seenProperties.insert(propType).second)
				throw std::runtime_error("Duplicate property modification \"" +
										 std::string(Config::unitPropertyToString(propType)) + "\" in component \"" +
										 comp.id + "\".");
			comp.properties[propType] = modification;
		}
		comp.cost = componentJson.at("cost").get<unsigned int>();
		config.componentTypes.push_back(comp);
	}

	for (const auto &conditionJson : components.at("invalidConditions"))
	{
		InvalidConditionConfig condition;
		condition.message = conditionJson.at("message").get<std::string>();
		condition.condition = conditionJson.at("condition");
		validateInvalidConditionExpr(condition.condition);
		config.invalidConditions.push_back(std::move(condition));
	}

	for (const auto &posJson : j.at("corePositions"))
	{
		Position pos;
		pos.x = posJson.at("x").get<int>();
		pos.y = posJson.at("y").get<int>();

		if (!pos.isValid(config.gridSize))
		{
			Logger::LogErr("Invalid core position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ").");
			exit(EXIT_FAILURE);
		}

		config.corePositions.push_back(pos);
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
ComponentConfig *Config::getComponentConfig(const std::string &id)
{
	for (auto &component : game().componentTypes)
	{
		if (component.id == id) return &component;
	}

	return nullptr;
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
