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
	{ObjectType::Deposit, 'R'},
	{ObjectType::Wall, '#'},
	{ObjectType::GemPile, '$'},
	{ObjectType::Bomb, '*'}};

static constexpr char EMPTY_CELL = '.';

void Visualizer::visualizeGameState(unsigned long long tick, bool force)
{
	if (!Config::server().enableTerminalVisualizer && !force)
		return;


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
			  << "    Deposit  = '" << OBJ_SYMBOL.at(ObjectType::Deposit) << "'\n"
			  << "    Wall     = '" << OBJ_SYMBOL.at(ObjectType::Wall) << "'\n"
			  << "    GemPile  = '" << OBJ_SYMBOL.at(ObjectType::GemPile) << "'\n"
			  << "    Bomb     = '" << OBJ_SYMBOL.at(ObjectType::Bomb) << "'\n"
			  << "    Empty    = '" << EMPTY_CELL << "'\n\n";

	for (const Object & obj : Board::instance())
	{
		ObjectType type = obj.getType();
		if (type == ObjectType::Bomb || type == ObjectType::GemPile || type == ObjectType::Deposit || type == ObjectType::Wall)
			continue;
		std::cout << "ID: " << obj.getId() << " HP: " << obj.getHP();
		if (obj.getType() == ObjectType::Core)
		{
			const Core &core = dynamic_cast<const Core &>(obj);
			std::cout << " Type: Core";
			std::cout << " Team: " << core.getTeamId();
			std::cout << " gems: " << core.getBalance();
		}
		if  (obj.getType() == ObjectType::Unit)
		{
			const Unit &unit = dynamic_cast<const Unit &>(obj);
			std::cout << " Type: Unit";
			std::cout << " Team: " << unit.getTeamId();
			std::cout << " Type: " << unit.getUnitType();
			std::cout << " gems: " << unit.getBalance();
			std::cout << " Next Move Opp: " << unit.getActionCooldown();
		}
		std::cout << std::endl;
	}

	Core * core0 = Board::instance().getCoreByTeamId(1);
	Core * core1 = Board::instance().getCoreByTeamId(2);
	if (core0)
		std::cout << "Core H Health: " << core0->getHP() << std::endl;
	if (core1)
		std::cout << "Core h Health: " << core1->getHP() << std::endl;

	int H = Config::game().gridSize;
	int W = Config::game().gridSize;

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
