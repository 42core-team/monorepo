#ifndef OBJECT_H
#define OBJECT_H

struct Position
{
	unsigned int x;
	unsigned int y;
};

class Object
{
	public:
		virtual unsigned int getId() const = 0;
		virtual unsigned int getTeamId() const = 0;
		virtual Position getPosition() const = 0;
		virtual unsigned int getHP() const = 0;
		virtual bool dealDamage(int dmg) = 0;
		virtual ~Object() {}
};

#endif // OBJECT_H
