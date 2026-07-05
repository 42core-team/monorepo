#ifndef CONFIG_H
#define CONFIG_H

#include "Common.h"
#include "json.hpp"

#include <string>
#include <vector>
using json = nlohmann::ordered_json;

class WorldGenerator;

struct UnitConfig;
struct ComponentConfig;
enum class UnitProperty;
struct InvalidConditionConfig;

struct GameConfig
{
	std::unique_ptr<WorldGenerator> worldGenerator;
	json worldGeneratorConfig;

	unsigned int gridSize;

	uint64_t seed;
	std::string seedString = "";
	bool usedRandomSeed = false;

	unsigned int idleIncome;		// idle income per tick
	unsigned int idleIncomeTimeOut; // idle income duration in ticks

	unsigned int depositHp;
	unsigned int depositIncome;
	unsigned int gemPileIncome;

	unsigned int coreHp;
	unsigned int initialBalance;

	unsigned int wallHp;

	unsigned int maxComponentsPerUnit;
	unsigned int defaultUnitCost;
	std::map<UnitProperty, int> defaultUnitProperties;
	std::vector<ComponentConfig> componentTypes;
	std::vector<InvalidConditionConfig> invalidConditions;

	// core positions. length defines max supported player count
	std::vector<Position> corePositions;
};

enum class UnitProperty
{
	HP,
	BASE_ACTION_COOLDOWN,
	BALANCE_PER_COOLDOWN_STEP,
	MAX_BALANCE,
	DAMAGE_REDUCTION_PERCENT,
	DAMAGE_CORE,
	DAMAGE_UNIT,
	DAMAGE_OBJECT,
	POST_SPAWN_CORE_COOLDOWN
};
inline constexpr std::array<std::pair<std::string_view, UnitProperty>, 9> UNIT_PROPERTY_ENTRIES{{
		{"hp", UnitProperty::HP},
		{"baseActionCooldown", UnitProperty::BASE_ACTION_COOLDOWN},
		{"balancePerCooldownStep", UnitProperty::BALANCE_PER_COOLDOWN_STEP},
		{"maxBalance", UnitProperty::MAX_BALANCE},
		{"damageReductionPercent", UnitProperty::DAMAGE_REDUCTION_PERCENT},
		{"damageCore", UnitProperty::DAMAGE_CORE},
		{"damageUnit", UnitProperty::DAMAGE_UNIT},
		{"damageObject", UnitProperty::DAMAGE_OBJECT},
		{"postSpawnCoreCooldown", UnitProperty::POST_SPAWN_CORE_COOLDOWN},
}};

struct InvalidConditionConfig
{
	std::string message;
	json condition;
};

struct ComponentConfig
{
	std::string id;
	std::map<UnitProperty, int> properties;
	unsigned int cost;
};

struct ServerConfig
{
	std::vector<std::string> replayFolderPaths;
	unsigned int timeoutTicks;
	unsigned int timeoutMs;
	unsigned int clientWaitTimeoutMs;
	unsigned int clientConnectTimeoutMs;
	unsigned int clientPacketsMaxSizeKb;
};

class Config
{
  public:
	static GameConfig &game();
	static ServerConfig &server();

	static json encodeConfig();

	static Position &getCorePosition(unsigned int teamId);
	static ComponentConfig *getComponentConfig(const std::string &id);

	static void setServerConfigFilePath(const std::string &path) { serverConfigFilePath = path; }
	static std::string getServerConfigFilePath() { return serverConfigFilePath; }
	static void setGameConfigFilePath(const std::string &path) { gameConfigFilePath = path; }
	static std::string getGameConfigFilePath() { return gameConfigFilePath; }
	static void setDataFolderPath(const std::string &path)
	{
		dataFolderPath = path;
		if (dataFolderPath.back() == '/') dataFolderPath.pop_back();
	}
	static std::string getDataFolderPath() { return dataFolderPath; }

	static std::string_view unitPropertyToString(UnitProperty property);
	static UnitProperty stringToUnitProperty(std::string_view name);

	// misc utils
	static json load_json_schema(const std::string &schema_name);

  private:
	static std::string serverConfigFilePath;
	static std::string gameConfigFilePath;
	static std::string dataFolderPath;
};

#endif // CONFIG_H
