#ifndef UNIT_H
#define UNIT_H

#include <list>

#include "Board.h"
#include "Common.h"
#include "Config.h"
#include "Object.h"

class Unit : public Object
{
  public:
	Unit(unsigned int teamId, unsigned int unit_type);
	Unit(const Unit &other)
		: Object(other), unit_type_(other.unit_type_), team_id_(other.team_id_), balance_(other.balance_),
		  action_cooldown_(other.action_cooldown_)
	{
	}

	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);

	void tickActionCooldown();

	unsigned int getUnitType() const { return unit_type_; }
	unsigned int getTeamId() const { return team_id_; }
	unsigned int getBalance() const { return balance_; }
	unsigned int getActionCooldown() const { return action_cooldown_; }
	const std::list<Position> &getDebugPath() const { return debug_path_; }

	void setDebugPath(const std::list<Position> &path) { debug_path_ = path; }
	void addBalance(unsigned int amount) { balance_ += amount; }
	void setBalance(unsigned int amount) { balance_ = amount; }
	void resetActionCooldown() { action_cooldown_ = calcActionCooldown(); }

  private:
	unsigned int unit_type_;
	unsigned int team_id_;
	unsigned int balance_;
	unsigned int action_cooldown_ = 0;
	std::list<Position> debug_path_;

	unsigned int calcActionCooldown();
};

#endif // UNIT_H
