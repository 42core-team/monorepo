#include "Game.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

void Game::visualizeGameState(unsigned long long tick)
{
	std::cout << "Tick: " << tick << std::endl;

	std::cout << "Cores: " << "🏰 🏯" << std::endl;
	static const std::pair<std::string, std::string> CORE_EMOJI = {u8"🏰", u8"🏯"};

	std::cout << "Units: "
			  << "Warrior: 🤾‍♂️/🤾; Miner: 🏋️‍♂️/🏋️; Carrier: 🤹‍♂️/🤹; "
				 "Builder: 👷‍♂️/👷; Bomberman: 👨‍🎤/🧑‍🎤"
			  << std::endl;
	static const std::vector<std::pair<std::string, std::string>> UNIT_EMOJI = {
		{u8"🤾‍♂️", u8"🤾"},
		{u8"🏋️‍♂️", u8"🏋️"},
		{u8"🤹‍♂️", u8"🤹"},
		{u8"👷‍♂️", u8"👷"},
		{u8"👨‍🎤", u8"🧑‍🎤"}};

	std::cout << "Objects: " << "Resource: 💎; Wall: 🧱; Money: 💰; Bomb: 💣" << std::endl;
	static const std::unordered_map<ObjectType, std::string> SINGLE_EMOJI = {
		{ObjectType::Resource, u8"💎"},
		{ObjectType::Wall, u8"🧱"},
		{ObjectType::Money, u8"💰"},
		{ObjectType::Bomb, u8"💣"}};

	static const std::string EMPTY_CELL = u8"\u3000";

	int H = Config::getInstance().height;
	int W = Config::getInstance().width;

	std::cout << u8"╔";
	for (int x = 0; x < W; ++x)
		std::cout << u8"═" << u8"═";
	std::cout << u8"╗\n";

	for (int y = 0; y < H; ++y)
	{
		std::cout << u8"║";
		for (int x = 0; x < W; ++x)
		{
			auto pos = Position(x, y);
			Object *obj = board_.getObjectAtPos(pos);
			if (!obj)
			{
				std::cout << EMPTY_CELL;
				continue;
			}

			switch (obj->getType())
			{
			case ObjectType::Core:
			{
				Core *core = dynamic_cast<Core *>(obj);
				int team = core->getTeamId() % 2;
				std::cout << (team == 0 ? CORE_EMOJI.first : CORE_EMOJI.second);
				break;
			}

			case ObjectType::Unit:
			{
				Unit *unit = dynamic_cast<Unit *>(obj);
				int tid = unit->getUnitType();
				int team = unit->getTeamId() % 2;
				if (tid >= 0 && tid < (int)UNIT_EMOJI.size())
				{
					auto &tones = UNIT_EMOJI[tid];
					std::cout << (team == 0 ? tones.first : tones.second);
				}
				else
				{
					std::cout << u8"❓";
				}
				break;
			}

			default:
			{
				auto it = SINGLE_EMOJI.find(obj->getType());
				std::cout << (it != SINGLE_EMOJI.end() ? it->second : u8"❔");
			}
			}
		}
		std::cout << u8"║\n";
	}

	std::cout << u8"╚";
	for (int x = 0; x < W; ++x)
		std::cout << u8"═" << u8"═";
	std::cout << u8"╝\n\n";
}
