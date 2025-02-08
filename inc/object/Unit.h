#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"

class Unit : public Object
{
	public:
		Unit(unsigned int id, unsigned int teamId, Position pos, unsigned int type_id);
		
		void travel(MovementDirection dir);
		unsigned int getTypeId() const { return type_id_; }
		unsigned int getTeamId() const { return team_id_; }
		unsigned int getBalance() const { return balance_; }

		void addBalance(unsigned int amount) { balance_ += amount; }

	private:
		unsigned int type_id_;
		unsigned int team_id_;
		unsigned int balance_;
};

#endif // UNIT_H
