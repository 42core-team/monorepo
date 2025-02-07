#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"

class Unit : public Object
{
	public:
		Unit(unsigned int id, unsigned int teamId, Position pos, int hp, unsigned int type_id);
		
		void travel(MovementDirection dir);
		unsigned int getTypeId() const { return type_id_; }

	private:
		unsigned int type_id_;
};

#endif // UNIT_H
