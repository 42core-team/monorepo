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
		  balance_(other.balance_), move_cooldown_(other.move_cooldown_) {}

	void tick(unsigned long long tickCount);

	void tickMoveCooldown();

	unsigned int getUnitType() const { return unit_type_; }
	unsigned int getTeamId() const { return team_id_; }
	unsigned int getBalance() const { return balance_; }
	unsigned int getMoveCooldown() const { return move_cooldown_; }

	void addBalance(unsigned int amount) { balance_ += amount; }
	void setBalance(unsigned int amount) { balance_ = amount; }
	void resetMoveCooldown() { move_cooldown_ = calcMoveCooldown(); }

private:
	unsigned int unit_type_;
	unsigned int team_id_;
	unsigned int balance_;
	unsigned int move_cooldown_ = 0;

	unsigned int calcMoveCooldown();
};

#endif // UNIT_H
