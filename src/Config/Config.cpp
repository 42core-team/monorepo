#include "Config.h"

#include "JigsawWorldGenerator.h"
#include "DistancedResourceWorldGenerator.h"

namespace {
	std::unique_ptr<GameConfig> configInstance;
}

GameConfig Config::hardCoreConfig()
{
	GameConfig config;

	config.worldGenerator = std::make_unique<JigsawWorldGenerator>();

	config.width = 25;
	config.height = 25;
	config.tickRate = 5;

	config.idleIncome = 1;
	config.idleIncomeTimeOut = 600; // 2 minutes

	config.resourceHp = 50;
	config.resourceIncome = 200;

	config.coreHp = 350;
	config.initialBalance = 200;

	config.wallHp = 100;
	config.wallBuildCost = 20;

	// unit order must match order of units in conn lib unit type enum

	UnitConfig warrior;
	warrior.name = "Warrior";
	warrior.cost = 150;
	warrior.hp = 25;
	warrior.speed = 3;
	warrior.minSpeed = 12;
	warrior.damageCore = 10;
	warrior.damageUnit = 6;
	warrior.damageResource = 2;
	warrior.damageWall = 3;
	warrior.attackType = AttackType::DIRECT_HIT;
	warrior.attackReach = 1;
	warrior.canBuild = false;
	config.units.push_back(warrior);

	UnitConfig miner;
	miner.name = "Miner";
	miner.cost = 100;
	miner.hp = 15;
	miner.speed = 5;
	miner.minSpeed = 15;
	miner.damageCore = 3;
	miner.damageUnit = 2;
	miner.damageResource = 10;
	miner.damageWall = 5;
	miner.canBuild = false;
	miner.attackType = AttackType::DIRECT_HIT;
	miner.attackReach = 1;
	config.units.push_back(miner);

	UnitConfig carrier;
	carrier.name = "Carrier";
	carrier.cost = 125;
	carrier.hp = 20;
	carrier.speed = 3;
	carrier.minSpeed = 5;
	carrier.damageCore = 3;
	carrier.damageUnit = 2;
	carrier.damageResource = 4;
	carrier.damageWall = 3;
	carrier.attackType = AttackType::DIRECT_HIT;
	carrier.attackReach = 1;
	carrier.canBuild = false;
	config.units.push_back(carrier);

	UnitConfig builder;
	builder.name = "Builder";
	builder.cost = 300;
	builder.hp = 30;
	builder.speed = 2;
	builder.minSpeed = 8;
	builder.damageCore = 5;
	builder.damageUnit = 3;
	builder.damageResource = 2;
	builder.damageWall = 3;
	builder.attackType = AttackType::DIRECT_HIT;
	builder.attackReach = 1;
	builder.canBuild = true;
	config.units.push_back(builder);

	config.corePositions.push_back({ 0, 0 });   // top left
	config.corePositions.push_back({ 24, 24 }); // bottom right
	config.corePositions.push_back({ 24, 0 });  // top right
	config.corePositions.push_back({ 0, 24 });  // bottom left

	return config;
}

GameConfig Config::softCoreConfig()
{
	GameConfig config;

	config.worldGenerator = std::make_unique<DistancedResourceWorldGenerator>();

	config.width = 25;
	config.height = 25;
	config.tickRate = 5;

	config.idleIncome = 1;
	config.idleIncomeTimeOut = 600; // 2 minutes

	config.resourceHp = 50;
	config.resourceIncome = 200;

	config.coreHp = 350;
	config.initialBalance = 200;

	config.wallHp = 100;
	config.wallBuildCost = 20;

	// unit order must match order of units in conn lib unit type enum

	UnitConfig warrior;
	warrior.name = "Warrior";
	warrior.cost = 150;
	warrior.hp = 25;
	warrior.speed = 3;
	warrior.minSpeed = 10;
	warrior.damageCore = 10;
	warrior.damageUnit = 6;
	warrior.damageResource = 2;
	warrior.damageWall = 3;
	warrior.attackType = AttackType::DIRECT_HIT;
	warrior.attackReach = 1;
	warrior.canBuild = false;
	config.units.push_back(warrior);

	UnitConfig miner;
	miner.name = "Miner";
	miner.cost = 100;
	miner.hp = 15;
	miner.speed = 5;
	miner.minSpeed = 12;
	miner.damageCore = 3;
	miner.damageUnit = 2;
	miner.damageResource = 10;
	miner.damageWall = 5;
	miner.canBuild = false;
	miner.attackType = AttackType::DIRECT_HIT;
	miner.attackReach = 1;
	config.units.push_back(miner);

	UnitConfig carrier;
	carrier.name = "Carrier";
	carrier.cost = 125;
	carrier.hp = 20;
	carrier.speed = 3;
	carrier.minSpeed = 3;
	carrier.damageCore = 3;
	carrier.damageUnit = 2;
	carrier.damageResource = 4;
	carrier.damageWall = 3;
	carrier.attackType = AttackType::DIRECT_HIT;
	carrier.attackReach = 1;
	carrier.canBuild = false;
	config.units.push_back(carrier);

	config.corePositions.push_back({ 0, 0 });   // top left
	config.corePositions.push_back({ 24, 24 }); // bottom right
	config.corePositions.push_back({ 24, 0 });  // top right
	config.corePositions.push_back({ 0, 24 });  // bottom left
	config.corePositions.push_back({ 0, 12 });  // left
	config.corePositions.push_back({ 24, 12 }); // right
	config.corePositions.push_back({ 12, 0 });  // top
	config.corePositions.push_back({ 12, 24 }); // bottom

	return config;
}

GameConfig & Config::getInstance()
{
	if (!configInstance)
		configInstance = std::make_unique<GameConfig>(hardCoreConfig());
	return *configInstance;
}
void Config::initConfig(bool softcore)
{
	if (!configInstance) {
		if (softcore)
			configInstance = std::make_unique<GameConfig>(softCoreConfig());
		else
			configInstance = std::make_unique<GameConfig>(hardCoreConfig());
	}
}

Position & Config::getCorePosition(unsigned int teamId)
{
	return getInstance().corePositions[teamId];
}
UnitConfig & Config::getUnitConfig(unsigned int typeId)
{
	return getInstance().units[typeId];
}
