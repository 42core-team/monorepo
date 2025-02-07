#include "Config.h"

GameConfig Config::defaultConfig()
{
	GameConfig config;

	config.width = 50;
	config.height = 50;
	config.tickRate = 5;

	config.idleIncome = 1;
	config.idleIncomeTimeOut = 600; // 2 minutes

	config.resourceHp = 50;
	config.resourceIncome = 5;

	config.coreHp = 200;
	config.initialBalance = 200;

	UnitConfig warrior;
	warrior.name = "Warrior";
	warrior.cost = 150;
	warrior.hp = 25;
	warrior.speed = 3;
	warrior.damageCore = 10;
	warrior.damageUnit = 6;
	warrior.damageResource = 2;
	config.units.push_back(warrior);

	UnitConfig miner;
	miner.name = "Miner";
	miner.cost = 100;
	miner.hp = 15;
	miner.speed = 5;
	miner.damageCore = 3;
	miner.damageUnit = 2;
	miner.damageResource = 10;
	config.units.push_back(miner);

	config.corePositions.push_back({ 0, 0 });   // top left
	config.corePositions.push_back({ 49, 49 }); // bottom right
	config.corePositions.push_back({ 49, 0 });  // top right
	config.corePositions.push_back({ 0, 49 });  // bottom left
	config.corePositions.push_back({ 0, 24 });  // left
	config.corePositions.push_back({ 49, 24 }); // right
	config.corePositions.push_back({ 24, 0 });  // top
	config.corePositions.push_back({ 24, 49 }); // bottom

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
