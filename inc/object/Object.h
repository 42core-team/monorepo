#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Config.h"

enum class ObjectType
{
	Core,
	Unit
};

class Object
{
	public:
		Object(unsigned int id, unsigned int teamId, Position pos, int hp, ObjectType type)
			: id_(id), teamId_(teamId), position_(pos), hp_(hp), type_(type) {}
		virtual ~Object() {}

		unsigned int getId() const { return id_; };
		unsigned int getTeamId() const { return teamId_; };
		Position getPosition() const { return position_; };
		int getHP() const { return hp_; };
		ObjectType getType() const { return type_; };

		void setPosition(Position pos) { position_ = pos; };
		void setHP(int hp) { hp_ = hp; };

	protected:
		unsigned int id_;
		unsigned int teamId_;
		Position position_;
		int hp_;

		ObjectType type_;
};

#endif // OBJECT_H
