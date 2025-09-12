#ifndef CORE_H
#define CORE_H

#include "Common.h"
#include "Config.h"
#include "Object.h"

class Core : public Object
{
  public:
	Core(unsigned int teamId);
	Core(const Core &other) : Object(other), balance_(other.balance_), team_id_(other.team_id_), spawn_cooldown_(0) {}
	Core &operator=(const Core &other)
	{
		if (this == &other) return *this;
		Object::operator=(other);
		balance_ = other.balance_;
		team_id_ = other.team_id_;
		spawn_cooldown_ = other.spawn_cooldown_;
		return *this;
	}

	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);

	void tickSpawnCooldown();

	unsigned int getBalance() const { return balance_; }
	void setBalance(unsigned int gems) { balance_ = gems; }
	unsigned int getTeamId() const { return team_id_; }

	unsigned int getSpawnCooldown() const { return spawn_cooldown_; }
	void setSpawnCooldown(unsigned int cooldown) { spawn_cooldown_ = cooldown; }

  private:
	unsigned int balance_;
	unsigned int team_id_;
	unsigned int spawn_cooldown_;
};

#endif // CORE_H
