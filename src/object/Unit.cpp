#include "Unit.h"

Unit::Unit(unsigned int id, unsigned int teamId, Position pos, int hp, unsigned int type_id)
	: Object(id, teamId, pos, hp)
{
	type_id_ = type_id;
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
			if (position_.y < config.height - 1)
				position_.y += 1;
			break;
		case MovementDirection::LEFT:
			if (position_.x > 0)
				position_.x -= 1;
			break;
		case MovementDirection::RIGHT:
			if (position_.x < config.width - 1)
				position_.x += 1;
			break;
	}
}
