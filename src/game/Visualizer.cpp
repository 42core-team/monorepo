#include "Game.h"

#include <string>

void Game::visualizeGameState()
{
	std::cout << "Core: 🏛️; Units: W / w = Warrior; M / m = Miner; . = empty; 💎 = Resource; ⬛️ = Wall" << std::endl;

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
					std::cout << "🏛️";
				}
				else if (obj->getType() == ObjectType::Unit)
				{
					Unit * unit = dynamic_cast<Unit*>(obj);
					std::string unitInd = "??";
					if (unit->getTypeId() == 0)
						unitInd = "Ww";
					else if (unit->getTypeId() == 1)
						unitInd = "Mm";
					std::cout << unitInd[unit->getTeamId() % 2];
				}
				else if (obj->getType() == ObjectType::Resource)
				{
					std::cout << "💎";
				}
				else if (obj->getType() == ObjectType::Wall)
				{
					std::cout << "⬛️";
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
