#ifndef DEPOSIT_H
#define DEPOSIT_H

#include "Board.h"
#include "Common.h"
#include "Config.h"
#include "GemPile.h"
#include "Object.h"
#include "Unit.h"

#include <cmath>

class Deposit : public Object
{
  public:
	Deposit();
	Deposit(unsigned int gems);
	Deposit(const Deposit &other) : Object(other), balance_(other.balance_) {}

	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);

	unsigned int getBalance() const { return balance_; }
	void setBalance(unsigned int gems) { balance_ = gems; }

  private:
	unsigned int balance_;
};

#endif // DEPOSIT_H
