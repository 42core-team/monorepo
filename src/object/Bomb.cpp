#include "Bomb.h"

Bomb::Bomb(unsigned int id, Position pos)
	: Object(id, pos, Config::instance().bombHp, ObjectType::Bomb), countdown_(Config::instance().bombCountdown) {}

void Bomb::tick(unsigned long long tickCount)
{
	(void)tickCount;
	countdown_--;

	if (countdown_ == 0)
	{
		int bombReach = Config::instance().bombReach;
		for (int i = 0; i < bombReach; i++) // pos X
		{
			Object *obj = Board::instance().getObjectAtPos(Position(position_.x + i, position_.y));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::instance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // pos Y
		{
			Object *obj = Board::instance().getObjectAtPos(Position(position_.x, position_.y + i));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::instance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // neg X
		{
			Object *obj = Board::instance().getObjectAtPos(Position(position_.x - i, position_.y));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::instance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // neg Y
		{
			Object *obj = Board::instance().getObjectAtPos(Position(position_.x, position_.y - i));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::instance().bombDamage);
		}
		Board::instance().removeObjectById(this->getId());
	}
}
