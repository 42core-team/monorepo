#ifndef CONFIG_H
#define CONFIG_H

#include "Common.h"
#include "json.hpp"

#include <memory>
#include <string>
#include <vector>
using json = nlohmann::ordered_json;

class WorldGenerator;

struct UnitConfig;

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
	unsigned int wallBuildCost;

	unsigned int bombHp;
	unsigned int bombCountdown;
	unsigned int bombThrowCost;
	unsigned int bombReach;
	unsigned int bombDamageCore;
	unsigned int bombDamageUnit;
	unsigned int bombDamageDeposit;

	std::vector<UnitConfig> units;

	// core positions. length defines max supported player count
	std::vector<Position> corePositions;
};

enum class BuildType
{
	NONE,
	BOMB,
	WALL
};

struct UnitConfig
{
	std::string name;
	unsigned int cost;
	unsigned int hp;
	unsigned int baseActionCooldown; // timeout between actions in ticks
	unsigned int maxActionCooldown;
	unsigned int balancePerCooldownStep; // action cooldown = base action cooldown + gems / balancePerCooldownStep

	unsigned int damageCore;
	unsigned int damageUnit;
	unsigned int damageDeposit;
	unsigned int damageWall;
	unsigned int damageBomb;

	BuildType buildType;
};

struct ServerConfig
{
	std::vector<std::string> replayFolderPaths;
	unsigned int timeoutTicks;
	unsigned int timeoutMs;
	unsigned int clientWaitTimeoutMs;
	unsigned int clientConnectTimeoutMs;
	bool enableTerminalVisualizer;
};

class Config
{
  public:
	static GameConfig &game();
	static ServerConfig &server();

	static json encodeConfig();

	static Position &getCorePosition(unsigned int teamId);
	static UnitConfig &getUnitConfig(unsigned int typeId);

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

  private:
	static std::string serverConfigFilePath;
	static std::string gameConfigFilePath;
	static std::string dataFolderPath;
};

#endif // CONFIG_H
