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

	unsigned int width;
	unsigned int height;
	unsigned int tickRate; // ticks per second
	unsigned int timeout;

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
	unsigned int speed; // timeout between moves in ticks
	unsigned int minSpeed;

	unsigned int damageCore;
	unsigned int damageUnit;
	unsigned int damageResource;
	unsigned int damageWall;
	unsigned int damageBomb;

	BuildType buildType;
};

class Config
{
	public:
		static GameConfig &instance();
		static json encodeConfig();

		static Position &getCorePosition(unsigned int teamId);
		static UnitConfig &getUnitConfig(unsigned int typeId);

		static void setConfigFilePath(const std::string &path)
		{
			configFilePath = path;
		}
		static std::string getConfigFilePath() { return configFilePath; }

	private:
		static std::string configFilePath;
};

#endif // CONFIG_H
