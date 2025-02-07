#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Config.h"

class Object
{
	public:
		Object(unsigned int id, unsigned int teamId, Position pos, int hp)
			: id_(id), teamId_(teamId), position_(pos), hp_(hp) {}

		unsigned int getId() const { return id_; };
		unsigned int getTeamId() const { return teamId_; };
		Position getPosition() const { return position_; };
		int getHP() const { return hp_; };
		bool takeDamage(int dmg) { hp_ -= dmg; return hp_ <= 0; };
		virtual ~Object() {}

	protected:
		unsigned int id_;
		unsigned int teamId_;
		Position position_;
		int hp_;
};

#endif // OBJECT_H
