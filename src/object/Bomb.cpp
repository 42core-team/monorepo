#include "Bomb.h"

Bomb::Bomb(unsigned int id)
	: Object(id, Config::game().bombHp, ObjectType::Bomb), countdown_(Config::game().bombCountdown) {}

void Bomb::explode()
{
	int bombReach = Config::game().bombReach;
	Position bombPos = Board::instance().getObjectPositionById(this->getId());
	for (int i = 0; i < bombReach; i++) // pos X
	{
		Object *obj = Board::instance().getObjectAtPos(Position(bombPos.x + i, bombPos.y));
		if (!obj)
			continue;
		if (obj->getType() == ObjectType::Wall)
			break;
		obj->setHP(obj->getHP() - Config::game().bombDamage);
	}
	for (int i = 0; i < bombReach; i++) // pos Y
	{
		Object *obj = Board::instance().getObjectAtPos(Position(bombPos.x, bombPos.y + i));
		if (!obj)
			continue;
		if (obj->getType() == ObjectType::Wall)
			break;
		obj->setHP(obj->getHP() - Config::game().bombDamage);
	}
	for (int i = 0; i < bombReach; i++) // neg X
	{
		Object *obj = Board::instance().getObjectAtPos(Position(bombPos.x - i, bombPos.y));
		if (!obj)
			continue;
		if (obj->getType() == ObjectType::Wall)
			break;
		obj->setHP(obj->getHP() - Config::game().bombDamage);
	}
	for (int i = 0; i < bombReach; i++) // neg Y
	{
		Object *obj = Board::instance().getObjectAtPos(Position(bombPos.x, bombPos.y - i));
		if (!obj)
			continue;
		if (obj->getType() == ObjectType::Wall)
			break;
		obj->setHP(obj->getHP() - Config::game().bombDamage);
	}
	Board::instance().removeObjectById(this->getId());
}

void Bomb::tick(unsigned long long tickCount)
{
	(void)tickCount;
	countdown_--;
	if (countdown_ == 0)
		explode();
}
