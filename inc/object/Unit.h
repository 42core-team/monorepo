#ifndef UNIT_H
#define UNIT_H

#include "Object.h"

class Unit : public Object
{
	public:
		Unit(unsigned int id, unsigned int teamId, Position pos, unsigned int hp);
		unsigned int getId() const override;
		unsigned int getTeamId() const override;
		Position getPosition() const override;
		unsigned int getHP() const override;
		bool dealDamage(int dmg) override;
		
		void travel(int dx, int dy);

	private:
		unsigned int id_;
		unsigned int teamId_;
		Position pos_;
		unsigned int hp_;
};

#endif // UNIT_H
