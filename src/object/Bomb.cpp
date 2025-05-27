#include "Bomb.h"

#include "Game.h"

Bomb::Bomb(unsigned int id, Position pos)
	: Object(id, pos, Config::getInstance().bombHp, ObjectType::Bomb), countdown_(Config::getInstance().bombCountdown) {}

void Bomb::tick(unsigned long long tickCount, Game * game)
{
	(void) tickCount;
	countdown_--;

	if (countdown_ == 0) {
		int bombReach = Config::getInstance().bombReach;
		for (int i = -bombReach; i <= bombReach; i++) {
			Position pos = Position(position_.x + i, position_.y);
			Object * obj = game->getObjectAtPos(pos);
			if (obj) {
				obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
			}
		}
		for (int i = -bombReach; i <= bombReach; i++) {
			Position pos = Position(position_.x, position_.y + i);
			Object * obj = game->getObjectAtPos(pos);
			if (obj) {
				obj->setHP(obj->getHP() - Config::getInstance().bombDamage);
			}
		}
		game->removeObjectById(this->getId());
	}
}

std::unique_ptr<Object> & Bomb::clone(Position newPos, Game * game) const
{
	int nextObjId = game->getNextObjectId();
	std::unique_ptr<Object> obj = std::make_unique<Bomb>(nextObjId, newPos);
	obj->setHP(this->getHP());
	game->getObjects().push_back(std::move(obj));
	return game->getObjects().back();
}
