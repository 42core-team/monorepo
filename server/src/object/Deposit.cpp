#include "Deposit.h"

Deposit::Deposit() : Object(Config::game().depositHp, ObjectType::Deposit), balance_(Config::game().depositIncome)
{
}
Deposit::Deposit(unsigned int gems) : Object(Config::game().depositHp, ObjectType::Deposit), balance_(gems)
{
}

void Deposit::tick(unsigned long long tickCount)
{
	(void)tickCount;
}

void Deposit::damage(Object *attacker, unsigned int damage)
{
	(void)attacker;

	Stats::instance().inc(stat_keys::damage_deposits, damage);

	this->setHP(this->getHP() - damage);
	if (this->getHP() > 0) return;

	Stats::instance().inc(stat_keys::deposits_destroyed, 1);

	int balance = this->getBalance();
	Position pos = Board::instance().getObjectPositionById(this->getId());
	Board::instance().removeObjectById(this->getId());
	Board::instance().addObject<GemPile>(GemPile(balance), pos);
}
