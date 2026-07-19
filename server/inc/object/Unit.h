#ifndef UNIT_H
#define UNIT_H

#include "Board.h"
#include "Common.h"
#include "Config.h"
#include "Object.h"

class Unit : public Object
{
  public:
	Unit(unsigned int teamId, std::map<UnitProperty, int> properties, std::vector<std::string> components,
		 std::string name);
	Unit(const Unit &other)
		: Object(other), properties_(other.properties_), components_(other.components_), name_(other.name_),
		  team_id_(other.team_id_), balance_(other.balance_), action_cooldown_(other.action_cooldown_)
	{
	}

	void tick(unsigned long long tickCount);
	void damage(Object *attacker, unsigned int damage);

	void tickActionCooldown();

	std::map<UnitProperty, int> getProperties() const { return properties_; }
	std::vector<std::string> getComponents() const { return components_; }
	const std::string &getName() const { return name_; }

	unsigned int getTeamId() const { return team_id_; }
	unsigned int getBalance() const { return balance_; }
	unsigned int getRemainingBalanceCapacity() const
	{
		const int configuredMaxBalance = properties_.at(UnitProperty::MAX_BALANCE);
		if (configuredMaxBalance <= 0) return 0;
		const unsigned int maxBalance = static_cast<unsigned int>(configuredMaxBalance);
		return balance_ < maxBalance ? maxBalance - balance_ : 0;
	}
	long long getActionCooldown() const { return action_cooldown_; }

	bool isOwnedByTeam(unsigned int teamId) const override { return teamId == team_id_; }

	void addBalance(unsigned int amount) { balance_ += amount; }
	void setBalance(unsigned int amount) { balance_ = amount; }
	void resetActionCooldown() { action_cooldown_ = calcActionCooldown(); }

  private:
	std::map<UnitProperty, int> properties_;
	std::vector<std::string> components_;
	std::string name_;
	unsigned int team_id_;
	unsigned int balance_;
	long long action_cooldown_ = 0;

	unsigned int calcActionCooldown();
};

#endif // UNIT_H
