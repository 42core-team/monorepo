#ifndef CORE_H
#define CORE_H

#include "Object.h"
#include "Common.h"

class Core : public Object
{
	public:
		Core(unsigned int id, unsigned int teamId, Position pos);

		void tick(unsigned long long tickCount);
		std::unique_ptr<Object> & clone(Position newPos, Game * game) const;

		unsigned int getBalance() const { return balance_; }
		void setBalance(unsigned int balance) { balance_ = balance; }
		unsigned int getTeamId() const { return team_id_; }

	private:
		unsigned int balance_;
		unsigned int team_id_;
};

#endif // CORE_H
