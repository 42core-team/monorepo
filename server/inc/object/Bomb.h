#ifndef BOMB_H
#define BOMB_H

#include "Board.h"
#include "Common.h"
#include "Config.h"
#include "Object.h"
#include "Stats.h"

#include <set>

class Bomb : public Object
{
  public:
	Bomb();
	Bomb(const Bomb &other) : Object(other), countdown_(other.countdown_), countdownStarted_(other.countdownStarted_) {}

	void explode();
	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);

	unsigned int getCountdown() const { return countdown_; }
	bool isCountdownStarted() const { return countdownStarted_; }

	std::set<Position> explosionTiles_;

  private:
	unsigned int countdown_ = 0; // Countdown for the bomb to explode
	bool countdownStarted_ = false;
};

#endif // BOMB_H
