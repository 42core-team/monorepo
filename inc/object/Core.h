#ifndef CORE_H
#define CORE_H

#include "Object.h"

class Core : public Object
{
	public:
		Core(unsigned int id, unsigned int teamId, Position pos);
};

#endif // CORE_H
