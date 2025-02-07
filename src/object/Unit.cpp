#include "Unit.h"

Unit::Unit(unsigned int id, unsigned int teamId, Position pos, unsigned int hp)
	: id_(id), teamId_(teamId), pos_(pos), hp_(hp) {}

unsigned int Unit::getId() const
{ return id_; }

unsigned int Unit::getTeamId() const
{ return teamId_; }

Position Unit::getPosition() const
{ return pos_; }

unsigned int Unit::getHP() const
{ return hp_; }

bool Unit::dealDamage(int dmg)
{
	if (dmg < 0) return false;
	if (dmg >= (int)hp_) {
		hp_ = 0;
		return true;
	}
	hp_ -= dmg;
	return false;
}

void Unit::travel(int dx, int dy)
{
	pos_.x += dx;
	pos_.y += dy;
}
