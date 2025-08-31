#ifndef WALL_H
#define WALL_H

#include "Object.h"
#include "Common.h"
#include "Board.h"
#include "Config.h"

class Wall : public Object
{
	public:
		Wall();
		Wall(const Wall &other)
			: Object(other) {}

		void tick(unsigned long long tickCount);
		void damage(Object *attacker, unsigned int damage);
};

#endif // WALL_H
