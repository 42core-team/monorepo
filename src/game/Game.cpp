#include "Game.h"

#include <chrono>
#include <iostream>

Game::Game(std::vector<unsigned int> team_ids) : teamCount_(team_ids.size()), nextObjectId_(1)
{
	GameConfig config = Config::getInstance();
	cores_.reserve(team_ids.size());
	for (unsigned int i = 0; i < team_ids.size(); ++i)
		cores_.push_back(Core(nextObjectId_++, team_ids[i], Config::getCorePosition(i)));
}

void Game::addBridge(Bridge* bridge)
{
	bridges_.push_back(bridge);
}

void Game::run()
{
	using clock = std::chrono::steady_clock;
	unsigned int ticksPerSecond = Config::getInstance().tickRate;
	auto tickInterval = std::chrono::nanoseconds(1000000000 / ticksPerSecond);

	auto startTime = clock::now();
	unsigned long long tickCount = 0;

	while (true) // CORE GAMELOOP
	{
		auto now = clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - startTime);
		unsigned long long expectedTickCount = elapsed.count() / tickInterval.count();

		if (expectedTickCount > tickCount)
		{
			// Over one tick passed, we're lagging.
			if (expectedTickCount > tickCount + 1)
			{
				unsigned long skipped = expectedTickCount - tickCount - 1;
				std::cerr << "[WARNING] Processing delay: skipping " 
							<< skipped << " tick(s)." << std::endl;
			}
			tickCount = expectedTickCount;
		}

		auto nextTickTime = startTime + (tickCount + 1) * tickInterval;
		std::this_thread::sleep_until(nextTickTime);

		tick();
		sendState();

		tickCount++;
	}
}

void Game::tick()
{
	// TODO
	std::cout << "Tick" << std::endl;
}

void Game::sendState()
{
	json state;

	state["cores"] = json::array();
	for (auto& core : cores_)
	{
		json c;

		c["id"] = core.getId();
		c["teamId"] = core.getTeamId();
		Position pos = core.getPosition();
		c["position"] = { {"x", pos.x}, {"y", pos.y} };
		c["hp"] = core.getHP();
		c["balance"] = core.getBalance();

		state["cores"].push_back(c);
	}

	state["units"] = json::array();
	for (auto& unit : units_)
	{
		json u;

		u["id"] = unit.getId();
		u["teamId"] = unit.getTeamId();
		Position pos = unit.getPosition();
		u["position"] = { {"x", pos.x}, {"y", pos.y} };
		u["hp"] = unit.getHP();
		u["type"] = unit.getTypeId();

		state["units"].push_back(u);
	}
	
	for (auto bridge : bridges_)
	{
		bridge->sendMessage(state);
	}
}
