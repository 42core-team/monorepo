#include "Config.h"

GameConfig Config::defaultConfig()
{
	GameConfig config;

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
	config.units.push_back(miner);

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
	static GameConfig instance = defaultConfig();
	return instance;
}

Position & Config::getCorePosition(unsigned int teamId)
{
	return getInstance().corePositions[teamId];
}

UnitConfig & Config::getUnitConfig(unsigned int typeId)
{
	return getInstance().units[typeId];
}
