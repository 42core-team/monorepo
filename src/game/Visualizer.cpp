#include "Game.h"

#include <string>

void Game::visualizeGameState(unsigned long long tick)
{
	std::cout << "Tick: " << tick << "; Core: C / c; Units: W / w = Warrior; M / m = Miner; . = empty; R = Resource; X = Wall" << std::endl;

	for (int y = 0; y < (int)Config::getInstance().height; y++)
	{
		for (int x = 0; x < (int)Config::getInstance().width; x++)
		{
			Position pos = Position(x, y);
			Object * obj = getObjectAtPos(pos);
			if (obj)
			{
				if (obj->getType() == ObjectType::Core)
				{
					Core * core = dynamic_cast<Core*>(obj);
					std::cout << "Cc"[core->getTeamId() % 2];
				}
				else if (obj->getType() == ObjectType::Unit)
				{
					Unit * unit = dynamic_cast<Unit*>(obj);
					std::string unitInd = "??";
					if (unit->getTypeId() == 0)
						unitInd = "Ww";
					else if (unit->getTypeId() == 1)
						unitInd = "Mm";
					else if (unit->getTypeId() == 2)
						unitInd = "Cc";
					else if (unit->getTypeId() == 3)
						unitInd = "Bb";
					else if (unit->getTypeId() == 4)
						unitInd = "Aa";
					std::cout << unitInd[unit->getTeamId() % 2];
				}
				else if (obj->getType() == ObjectType::Resource)
				{
					std::cout << "R";
				}
				else if (obj->getType() == ObjectType::Wall)
				{
					std::cout << "X";
				}
				else if (obj->getType() == ObjectType::Money)
				{
					std::cout << "$";
				}
			}
			else
			{
				std::cout << ".";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}
