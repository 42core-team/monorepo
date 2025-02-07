#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"

class Unit : public Object
{
	public:
		Unit(unsigned int id, unsigned int teamId, Position pos, int hp);
		
		void travel(MovementDirection dir);
};

#endif // UNIT_H
