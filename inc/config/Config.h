#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>
#include <memory>

#include "Common.h"

class WorldGenerator;

struct UnitConfig;

struct GameConfig
{
	std::unique_ptr<WorldGenerator> worldGenerator;

	unsigned int width;
	unsigned int height;
	unsigned int tickRate; // ticks per second

	unsigned int idleIncome; // idle income per tick
	unsigned int idleIncomeTimeOut; // idle income duration in ticks

	unsigned int resourceHp;
	unsigned int resourceIncome;

	unsigned int coreHp;
	unsigned int initialBalance;

	unsigned int wallHp;
	unsigned int wallBuildCost;

	std::vector<UnitConfig> units;

	// core positions. length defines max supported player count
	std::vector<Position> corePositions;
};

enum class AttackType
{
	DIRECT_HIT,
	DIRECTION_SHOT,
	AREA_DAMAGE // not implemented
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
	AttackType attackType;
	unsigned int attackReach;

	bool canBuild;
};

class Config
{
	public:
		static void initConfig(bool softcore);
		static GameConfig & getInstance();

		static GameConfig hardCoreConfig();
		static GameConfig softCoreConfig();

		static Position & getCorePosition(unsigned int teamId);
		static UnitConfig & getUnitConfig(unsigned int typeId);
};

#endif // CONFIG_H
