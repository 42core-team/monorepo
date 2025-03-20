#ifndef WALL_H
#define WALL_H

#include "Object.h"
#include "Common.h"

class Wall : public Object
{
	public:
		Wall(unsigned int id, Position pos);

		void tick(unsigned long long tickCount);
		std::unique_ptr<Object> & clone(Position newPos, Game * game) const;
};

#endif // WALL_H
