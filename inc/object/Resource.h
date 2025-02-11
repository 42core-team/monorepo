#ifndef RESOURCE_H
#define RESOURCE_H

#include "Object.h"
#include "Common.h"
#include "Unit.h"

class Resource : public Object
{
	public:
		Resource(unsigned int id, Position pos);

		void tick(unsigned long long tickCount);
		void getMined(Unit * miner);
};

#endif // RESOURCE_H
