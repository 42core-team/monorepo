#ifndef WALL_H
#define WALL_H

#include "Object.h"
#include "Common.h"
#include "Config.h"

class Wall : public Object
{
	public:
		Wall(unsigned int id, Position pos);
		Wall(const Wall &other)
			: Object(other) {}

		void tick(unsigned long long tickCount);
};

#endif // WALL_H
