#include "GemPile.h"

GemPile::GemPile()
	: Object(1, ObjectType::GemPile),
	balance_(Config::game().gemPileIncome) {}
GemPile::GemPile(unsigned int gems)
	: Object(1, ObjectType::GemPile),
	balance_(gems) {}

void GemPile::tick(unsigned long long tickCount)
{
	(void) tickCount;
}

void GemPile::damage(Object *attacker, unsigned int damage)
{
	(void) damage;

	if (attacker->getType() == ObjectType::Unit)
		((Unit *)attacker)->setBalance(((Unit *)attacker)->getBalance() + balance_);

	Stats::instance().inc(stat_keys::gems_gained, balance_);
	Stats::instance().inc(stat_keys::gempiles_destroyed);

	Board::instance().removeObjectById(id_);
}
