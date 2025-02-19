#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"
#include "Config.h"

// must be in same order as t_obj_type in connection lib
enum class ObjectType
{
	Core,
	Unit,
	Resource,
	Wall
};

class Object
{
	public:
		Object(unsigned int id, Position pos, int hp, ObjectType type)
			: id_(id), position_(pos), hp_(hp), type_(type) {}
		virtual ~Object() {}

		virtual void tick(unsigned long long tickCount) = 0;

		unsigned int getId() const { return id_; };
		Position getPosition() const { return position_; };
		int getHP() const { return hp_; };
		ObjectType getType() const { return type_; };

		void setPosition(Position pos) { position_ = pos; };
		void setHP(int hp) { hp_ = hp; };

	protected:
		unsigned int id_;
		Position position_;
		int hp_;

		ObjectType type_;
};

#endif // OBJECT_H
