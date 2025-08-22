#ifndef UNIT_H
#define UNIT_H

#include "Object.h"
#include "Common.h"
#include "Config.h"
#include "Board.h"

class Unit : public Object
{
public:
	Unit(unsigned int teamId, unsigned int unit_type);
	Unit(const Unit &other)
		: Object(other), unit_type_(other.unit_type_), team_id_(other.team_id_),
		  balance_(other.balance_), action_cooldown_(other.action_cooldown_) {}

	void tick(unsigned long long tickCount);

	void tickActionCooldown();

	unsigned int getUnitType() const { return unit_type_; }
	unsigned int getTeamId() const { return team_id_; }
	unsigned int getBalance() const { return balance_; }
	unsigned int getActionCooldown() const { return action_cooldown_; }

	void addBalance(unsigned int amount) { balance_ += amount; }
	void setBalance(unsigned int amount) { balance_ = amount; }
	void resetActionCooldown() { action_cooldown_ = calcActionCooldown(); }

private:
	unsigned int unit_type_;
	unsigned int team_id_;
	unsigned int balance_;
	unsigned int action_cooldown_ = 0;

	unsigned int calcActionCooldown();
};

#endif // UNIT_H
