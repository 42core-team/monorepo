#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"

// must be in same order as t_obj_type in connection lib
enum class ObjectType
{
	Core,
	Unit,
	Resource,
	Wall,
	Money,
	Bomb
};

class Object
{
	public:
		Object(unsigned int id, int hp, ObjectType type)
			: id_(id), hp_(hp), type_(type) {}
		Object(const Object &other)
			: id_(other.id_), hp_(other.hp_), type_(other.type_) {}
		virtual ~Object() {}

		virtual void tick(unsigned long long tickCount) = 0;

		unsigned int getId() const { return id_; };
		int getHP() const { return hp_; };
		ObjectType getType() const { return type_; };

		void setHP(int hp) { hp_ = hp; };

	protected:
		unsigned int id_;
		int hp_;

		ObjectType type_;
};

#endif // OBJECT_H
