#ifndef CONFIG_H
#define CONFIG_H

#include <vector>
#include <string>

#include "Common.h"

struct UnitConfig;

struct GameConfig
{
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

	std::vector<UnitConfig> units;

	// core positions. length defines max supported player count
	std::vector<Position> corePositions;
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
};

class Config
{
	public:
		static GameConfig defaultConfig();
		static GameConfig & getInstance();
		static Position & getCorePosition(unsigned int teamId);
		static UnitConfig & getUnitConfig(unsigned int typeId);
};


#endif // CONFIG_H
