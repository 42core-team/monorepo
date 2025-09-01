#include "HardcodedWorldGenerator.h"

HardcodedWorldGenerator::HardcodedWorldGenerator()
{
}

void HardcodedWorldGenerator::generateWorld(uint64_t seed)
{
	(void)seed;

	auto& rows = Config::game().worldGeneratorConfig["map"];
	std::string configWorldData;
	for (auto& row : rows)
		configWorldData += row.get<std::string>();
	if (configWorldData == "oh no")
	{
		Logger::LogErr("HardcodedWorldGenerator - No map data found in game config.");
		return;
	}
	configWorldData.erase(
		std::remove_if(
			configWorldData.begin(),
			configWorldData.end(),
			[](char c){ return c == '\n' || c == '\r'; }
		),
		configWorldData.end()
	);

	const unsigned int gridSize = Config::game().gridSize;

	if (configWorldData.size() != gridSize * gridSize)
	{
		Logger::LogWarn("HardcodedWorldGenerator - Map data size does not match grid size.");
	}

	for (size_t i = 0; i < configWorldData.size(); i++)
	{
		const unsigned int x = i % gridSize;
		const unsigned int y = i / gridSize;

		if (!Position(x, y).isValid(gridSize))
		{
			Logger::LogWarn("HardcodedWorldGenerator - Invalid obj at position (" + std::to_string(x) + ", " + std::to_string(y) + ") in map data.");
			continue;
		}

		for (size_t cp = 0; cp < Config::game().corePositions.size(); cp++)
		{
			if (Config::game().corePositions[cp].x == static_cast<int>(x) && \
				Config::game().corePositions[cp].y == static_cast<int>(y) && \
				configWorldData[i] != ' ')
			{
				Logger::LogWarn("HardcodedWorldGenerator - Found objects overlapping a core position at (" + std::to_string(x) + ", " + std::to_string(y) + ").");
				continue;
			}
		}

		switch (configWorldData[i])
		{
			case ' ':
				// Empty tile
				break;
			case 'W':
				// Wall
				Board::instance().addObject<Wall>(Wall(), Position(x, y), false);
				break;
			case 'R':
				// Deposit
				Board::instance().addObject<Deposit>(Deposit(), Position(x, y), false);
				break;
			case 'M':
				// GemPile
				Board::instance().addObject<GemPile>(GemPile(), Position(x, y), false);
				break;
			default:
				Logger::LogWarn("HardcodedWorldGenerator - Unknown tile type found in map data.");
				break;
		}
	}
}
