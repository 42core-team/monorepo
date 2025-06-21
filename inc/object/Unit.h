#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"
#include "Config.h"

class Unit : public Object
{
public:
	Unit(unsigned int id, unsigned int teamId, unsigned int unit_type);
	Unit(const Unit &other)
		: Object(other), unit_type_(other.unit_type_), team_id_(other.team_id_),
		  balance_(other.balance_), next_move_opp_(other.next_move_opp_) {}

	void tick(unsigned long long tickCount);

	unsigned int getUnitType() const { return unit_type_; }
	unsigned int getTeamId() const { return team_id_; }
	unsigned int getBalance() const { return balance_; }
	unsigned int getNextMoveOpp() const { return next_move_opp_; }

	void addBalance(unsigned int amount) { balance_ += amount; }
	void setBalance(unsigned int amount) { balance_ = amount; }
	void resetNextMoveOpp() { next_move_opp_ = calcNextMovementOpp(); }

private:
	unsigned int unit_type_;
	unsigned int team_id_;
	unsigned int balance_;
	unsigned int next_move_opp_;

	unsigned int calcNextMovementOpp();
};

#endif // UNIT_H
