#ifndef BOMB_H
#define BOMB_H

#include "Object.h"
#include "Common.h"

class Bomb : public Object
{
	public:
		Bomb(unsigned int id, Position pos);

		void tick(unsigned long long tickCount, Game * game);

		Position getPos() const { return pos_; }
		unsigned int getCountdown() const { return countdown_; }

	private:
		Position pos_;
		unsigned int countdown_ = 0; // Countdown for the bomb to explode
};

#endif // BOMB_H
