#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"

class Unit : public Object
{
	public:
		Unit(unsigned int id, unsigned int teamId, Position pos, unsigned int type_id);
		
		void tick(unsigned long long tickCount);
		std::unique_ptr<Object> & clone(Position newPos, Game * game) const;

		void travel(MovementDirection dir);
		bool canTravel() { return next_move_opp_ == 0; }

		unsigned int getTypeId() const { return type_id_; }
		unsigned int getTeamId() const { return team_id_; }
		unsigned int getBalance() const { return balance_; }
		unsigned int getNextMoveOpp() const { return next_move_opp_; }

		void addBalance(unsigned int amount) { balance_ += amount; }
		void setBalance(unsigned int amount) { balance_ = amount; }
		void resetNextMoveOpp() { next_move_opp_ = calcNextMovementOpp(); }

	private:
		unsigned int type_id_;
		unsigned int team_id_;
		unsigned int balance_;
		unsigned int next_move_opp_;

		unsigned int calcNextMovementOpp();
};

#endif // UNIT_H
