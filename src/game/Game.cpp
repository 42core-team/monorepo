#include "Game.h"

#include <chrono>
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
	// TODO
}
