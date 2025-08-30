#ifndef BOMB_H
#define BOMB_H

#include "Object.h"
#include "Common.h"
#include "Config.h"
#include "Board.h"
#include "Stats.h"

class Bomb : public Object
{
public:
	Bomb();
	Bomb(const Bomb &other)
		: Object(other), countdown_(other.countdown_), countdownStarted_(other.countdownStarted_) {}

	void explode();
	void tick(unsigned long long tickCount);

	unsigned int getCountdown() const { return countdown_; }

	void handleAttack();

private:
	unsigned int countdown_ = 0; // Countdown for the bomb to explode
	bool countdownStarted_ = false;
};

#endif // BOMB_H
