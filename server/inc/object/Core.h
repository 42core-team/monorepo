#ifndef CORE_H
#define CORE_H

#include "Object.h"
#include "Common.h"
#include "Config.h"

class Core : public Object
{
	public:
		Core(unsigned int teamId);
		Core (const Core & other)
			: Object(other), balance_(other.balance_), team_id_(other.team_id_) {}
		Core& operator=(const Core& other) {
			if (this == &other)
				return *this;
			Object::operator=(other);
			balance_ = other.balance_;
			team_id_ = other.team_id_;
			return *this;
		}

		void tick(unsigned long long tickCount);

		unsigned int getBalance() const { return balance_; }
		void setBalance(unsigned int balance) { balance_ = balance; }
		unsigned int getTeamId() const { return team_id_; }

	private:
		unsigned int balance_;
		unsigned int team_id_;
};

#endif // CORE_H
