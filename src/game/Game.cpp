#include "Game.h"

#include <chrono>
#include <cmath>
#include <iostream>

Game::Game(unsigned int teamCount) : teamCount_(teamCount), nextObjectId_(1)
{
	GameConfig config = Config::getInstance();
	cores_.reserve(teamCount);
	for (unsigned int i = 0; i < teamCount; ++i)
		cores_.push_back(Core(nextObjectId_++, i, Config::getCorePosition(i)));
}

void Game::addBridge(Bridge* bridge)
{
	bridges_.push_back(bridge);
}

void Game::run()
{
	// TODO
}

void Game::update()
{
	// TODO
}

void Game::sendState()
{
	// TODO
}
