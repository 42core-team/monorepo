#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <memory>

#include "Common.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class WorldGenerator;

struct UnitConfig;

struct GameConfig
{
	std::unique_ptr<WorldGenerator> worldGenerator;
	json worldGeneratorConfig;

	unsigned int gridSize;

	unsigned int seed = std::chrono::system_clock::now().time_since_epoch().count();

	unsigned int idleIncome;		// idle income per tick
	unsigned int idleIncomeTimeOut; // idle income duration in ticks

	unsigned int resourceHp;
	unsigned int resourceIncome;
	unsigned int moneyObjIncome;

	unsigned int coreHp;
	unsigned int initialBalance;

	unsigned int wallHp;
	unsigned int wallBuildCost;

	unsigned int bombHp;
	unsigned int bombCountdown;
	unsigned int bombThrowCost;
	unsigned int bombReach;
	unsigned int bombDamage;

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
	unsigned int baseMoveCooldown; // timeout between moves in ticks
	unsigned int maxMoveCooldown;

	unsigned int damageCore;
	unsigned int damageUnit;
	unsigned int damageResource;
	unsigned int damageWall;

	BuildType buildType;
};

struct ServerConfig
{
	std::string gameConfigFilePath;
	std::vector<std::string> replayFolderPaths;
	std::string dataFolderPath;
	unsigned int timeoutTicks;
	unsigned int timeoutMs;
	unsigned int clientWaitTimeoutMs;
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

		static void setServerConfigFilePath(const std::string &path)
		{
			serverConfigFilePath = path;
		}
		static std::string getServerConfigFilePath() { return serverConfigFilePath; }

	private:
		static std::string serverConfigFilePath;
};

#endif // CONFIG_H
