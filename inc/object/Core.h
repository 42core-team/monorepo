#ifndef CORE_H
#define CORE_H

#include "Object.h"

class Core : public Object
{
	public:
		Core(unsigned int id, unsigned int teamId, Position pos, unsigned int hp);
		unsigned int getId() const override;
		unsigned int getTeamId() const override;
		Position getPosition() const override;
		unsigned int getHP() const override;
		bool dealDamage(int dmg) override;

	private:
		unsigned int id_;
		unsigned int teamId_;
		Position pos_;
		unsigned int hp_;
};

#endif // CORE_H
