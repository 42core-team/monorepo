#ifndef OBJECT_H
#define OBJECT_H

#include "Common.h"

// must be in same order as t_obj_type in connection lib
enum class ObjectType
{
	Core,
	Unit,
	Deposit,
	Wall,
	GemPile,
	Bomb
};

class Object
{
	public:
		Object(int hp, ObjectType type)
			: id_(nextObjectId_++), hp_(hp), type_(type) {}
		Object(const Object &other)
			: id_(nextObjectId_++), hp_(other.hp_), type_(other.type_) {}
		Object& operator=(const Object& other) {
			if (this != &other) { hp_ = other.hp_; type_ = other.type_; }
			return *this;
		}
		virtual ~Object() {}

		virtual void tick(unsigned long long tickCount) = 0;

		unsigned int getId() const { return id_; };
		int getHP() const { return hp_; };
		ObjectType getType() const { return type_; };

		void setHP(int hp) { hp_ = hp; };

	protected:
		unsigned int id_;
		int hp_;

		static unsigned int nextObjectId_;

		ObjectType type_;
};

inline unsigned int Object::nextObjectId_ = 1;

#endif // OBJECT_H
