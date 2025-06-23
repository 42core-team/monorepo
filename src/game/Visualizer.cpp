#include "Visualizer.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

static const std::array<char, 2> CORE_SYMBOLS = {'H', 'h'};
static const std::array<char, 2> WARRIOR_SYM = {'W', 'w'};
static const std::array<char, 2> MINER_SYM = {'M', 'm'};
static const std::array<char, 2> CARRIER_SYM = {'C', 'c'};
static const std::array<char, 2> BUILDER_SYM = {'B', 'b'};
static const std::array<char, 2> BOMBER_SYM = {'Z', 'z'};

static const std::unordered_map<ObjectType, char> OBJ_SYMBOL = {
	{ObjectType::Resource, 'R'},
	{ObjectType::Wall, '#'},
	{ObjectType::Money, '$'},
	{ObjectType::Bomb, '*'}};

static constexpr char EMPTY_CELL = '.';

void Visualizer::visualizeGameState(unsigned long long tick)
{
	std::cout << "Tick: " << tick << "\n\n";

	std::cout << "  Cores:   Team0 = '" << CORE_SYMBOLS[0]
			  << "', Team1 = '" << CORE_SYMBOLS[1] << "'\n"
			  << "  Units:\n"
			  << "    Warrior:   Team0 = '" << WARRIOR_SYM[0]
			  << "', Team1 = '" << WARRIOR_SYM[1] << "'\n"
			  << "    Miner:     Team0 = '" << MINER_SYM[0]
			  << "', Team1 = '" << MINER_SYM[1] << "'\n"
			  << "    Carrier:   Team0 = '" << CARRIER_SYM[0]
			  << "', Team1 = '" << CARRIER_SYM[1] << "'\n"
			  << "    Builder:   Team0 = '" << BUILDER_SYM[0]
			  << "', Team1 = '" << BUILDER_SYM[1] << "'\n"
			  << "    Bomberman: Team0 = '" << BOMBER_SYM[0]
			  << "', Team1 = '" << BOMBER_SYM[1] << "'\n\n"
			  << "  Objects:\n"
			  << "    Resource = '" << OBJ_SYMBOL.at(ObjectType::Resource) << "'\n"
			  << "    Wall     = '" << OBJ_SYMBOL.at(ObjectType::Wall) << "'\n"
			  << "    Money    = '" << OBJ_SYMBOL.at(ObjectType::Money) << "'\n"
			  << "    Bomb     = '" << OBJ_SYMBOL.at(ObjectType::Bomb) << "'\n"
			  << "    Empty    = '" << EMPTY_CELL << "'\n\n";

	int H = Config::instance().height;
	int W = Config::instance().width;

	std::cout << "  ";
	for (int i = 0; i < W; ++i)
		std::cout << i % 10;
	std::cout << "\n";

	std::cout << " ╔";
	for (int x = 0; x < W; ++x)
		std::cout << "═";
	std::cout << "╗\n";

	for (int y = 0; y < H; ++y)
	{
		std::cout << y % 10;
		std::cout << "║";
		for (int x = 0; x < W; ++x)
		{
			auto pos = Position(x, y);
			Object *obj = Board::instance().getObjectAtPos(pos);
			if (!obj)
			{
				std::cout << EMPTY_CELL;
				continue;
			}

			char symbol = EMPTY_CELL;
			if (obj)
			{
				switch (obj->getType())
				{
				case ObjectType::Core:
				{
					Core *core = dynamic_cast<Core *>(obj);
					symbol = CORE_SYMBOLS[core->getTeamId() % 2];
					break;
				}

				case ObjectType::Unit:
				{
					Unit *unit = dynamic_cast<Unit *>(obj);
					int tid = unit->getUnitType();
					int team = unit->getTeamId() % 2;
					switch (tid)
					{
					case 0:
						symbol = WARRIOR_SYM[team];
						break;
					case 1:
						symbol = MINER_SYM[team];
						break;
					case 2:
						symbol = CARRIER_SYM[team];
						break;
					case 3:
						symbol = BUILDER_SYM[team];
						break;
					case 4:
						symbol = BOMBER_SYM[team];
						break;
					default:
						symbol = '?';
					}
					break;
				}

				default:
				{
					auto it = OBJ_SYMBOL.find(obj->getType());
					symbol = (it != OBJ_SYMBOL.end()
								  ? it->second
								  : '?');
					break;
				}
				}
			}

			std::cout << symbol;
		}
		std::cout << "║\n";
	}

	std::cout << " ╚";
	for (int x = 0; x < W; ++x)
		std::cout << "═";
	std::cout << "╝\n\n";
}
