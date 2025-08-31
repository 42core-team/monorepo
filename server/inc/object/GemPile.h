#ifndef GEM_PILE_H
#define GEM_PILE_H

#include "Object.h"
#include "Common.h"
#include "Unit.h"
#include "Config.h"
#include "Board.h"

#include <cmath>

class GemPile : public Object
{
	public:
		GemPile();
		GemPile(unsigned int gems);
		GemPile(const GemPile & other)
			: Object(other), balance_(other.balance_) {}

		void tick(unsigned long long tickCount);
		void damage(Object *attacker, unsigned int damage);

		unsigned int getBalance() const { return balance_; }
		void setBalance(unsigned int gems) { balance_ = gems; }

	private:
		unsigned int balance_;
};

#endif // GEM_PILE_H
