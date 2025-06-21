#ifndef BOMB_H
#define BOMB_H

#include "Object.h"
#include "Common.h"
#include "Config.h"
#include "Board.h"

class Bomb : public Object
{
	public:
		Bomb(unsigned int id);
		Bomb(const Bomb &other)
			: Object(other), countdown_(other.countdown_) {}

		void tick(unsigned long long tickCount);

		unsigned int getCountdown() const { return countdown_; }

	private:
		unsigned int countdown_ = 0; // Countdown for the bomb to explode
};

#endif // BOMB_H
