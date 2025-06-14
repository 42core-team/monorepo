#include "Bomb.h"

#include "Game.h"

Bomb::Bomb(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().bombHp, ObjectType::Bomb), countdown_(Config::getInstance().bombCountdown) {}

void Bomb::tick(unsigned long long tickCount, Game *game)
{
	(void)tickCount;
	countdown_--;

	if (countdown_ == 0)
	{
		int bombReach = Config::getInstance().bombReach;
		for (int i = 0; i < bombReach; i++) // pos X
		{
			Object *obj = game->getObjectAtPos(Position(position_.x + i, position_.y));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // pos Y
		{
			Object *obj = game->getObjectAtPos(Position(position_.x, position_.y + i));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // neg X
		{
			Object *obj = game->getObjectAtPos(Position(position_.x - i, position_.y));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
		}
		for (int i = 0; i < bombReach; i++) // neg Y
		{
			Object *obj = game->getObjectAtPos(Position(position_.x, position_.y - i));
			if (!obj)
				continue;
			if (obj->getType() == ObjectType::Wall)
				break;
			obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
		}
		game->removeObjectById(this->getId());
	}
}

std::unique_ptr<Object> &Bomb::clone(Position newPos, Game *game) const
{
	int nextObjId = game->getNextObjectId();
	std::unique_ptr<Object> obj = std::make_unique<Bomb>(nextObjId, newPos);
	obj->setHP(this->getHP());
	game->getObjects().push_back(std::move(obj));
	return game->getObjects().back();
}
