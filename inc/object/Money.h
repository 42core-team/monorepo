#ifndef MONEY_H
#define MONEY_H

#include "Object.h"
#include "Common.h"
#include "Unit.h"

#include <cmath>

class Money : public Object
{
	public:
		Money(unsigned int id, Position pos);
		Money(unsigned int id, Position pos, unsigned int balance);

		void tick(unsigned long long tickCount);
		std::unique_ptr<Object> & clone(Position newPos, Game * game) const;

		unsigned int getBalance() const { return balance_; }

	private:
		unsigned int balance_;
};

#endif // MONEY_H
