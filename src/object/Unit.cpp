#include "Unit.h"

#include "Game.h"

#include <cassert>

Unit::Unit(unsigned int id, unsigned int teamId, Position pos, unsigned int type_id)
	: Object(id, pos, Config::getInstance().units[type_id].hp, ObjectType::Unit), type_id_(type_id), team_id_(teamId), balance_(0)
{
	resetNextMoveOpp();
}

void Unit::travel(MovementDirection dir)
{
	GameConfig & config = Config::getInstance();

	switch (dir)
	{
		case MovementDirection::UP:
			if (position_.y > 0)
				position_.y -= 1;
			break;
		case MovementDirection::DOWN:
			if (position_.y < static_cast<int>(config.height) - 1)
				position_.y += 1;
			break;
		case MovementDirection::LEFT:
			if (position_.x > 0)
				position_.x -= 1;
			break;
		case MovementDirection::RIGHT:
			if (position_.x < static_cast<int>(config.width) - 1)
				position_.x += 1;
			break;
	}
}

void Unit::tick(unsigned long long tickCount)
{
	(void) tickCount;
	if (next_move_opp_ > 0)
		next_move_opp_--;
}

unsigned int Unit::calcNextMovementOpp()
{
	unsigned int baseSpeed = Config::getInstance().units[type_id_].speed;
	unsigned int minSpeed = Config::getInstance().units[type_id_].minSpeed;

	float resourcePart = balance_ / (Config::getInstance().resourceIncome / 4);
	if (resourcePart < 1)
		resourcePart = 1; // up to 1/4 resource balance does not slow down

	unsigned int speed = baseSpeed * resourcePart;

	if (speed > minSpeed)
		speed = minSpeed;

	return speed;
}

std::unique_ptr<Object> & Unit::clone(Position newPos, Game * game) const
{
	(void)newPos;
	(void)game;
	assert(false && "Unit::clone() should never be called for Core objects");
	static std::unique_ptr<Object> dummy;
	return dummy;
}
