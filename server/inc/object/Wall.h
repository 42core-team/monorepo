#ifndef WALL_H
#define WALL_H

#include "Board.h"
#include "Common.h"
#include "Config.h"
#include "Object.h"

class Wall : public Object
{
  public:
	Wall();
	Wall(const Wall &other) : Object(other) {}

	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);
};

#endif // WALL_H
