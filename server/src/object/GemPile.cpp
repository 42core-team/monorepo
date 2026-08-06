#include "GemPile.h"

GemPile::GemPile() : Object(1, ObjectType::GemPile), balance_(Config::game().gemPileIncome)
{
}
GemPile::GemPile(unsigned int gems) : Object(1, ObjectType::GemPile), balance_(gems)
{
}

void GemPile::tick(unsigned long long tickCount)
{
	(void)tickCount;
}

void GemPile::damage(Object *attacker, unsigned int damage)
{
	(void)damage;

	if (attacker->getType() != ObjectType::Unit) return;
	Unit *unitAttacker = (Unit *)attacker;

	const unsigned int collected = std::min(balance_, unitAttacker->getRemainingGemsCapacity());
	unitAttacker->addBalance(collected);
	balance_ -= collected;

	Stats::instance().inc(stat_keys::gems_gained, collected);
	if (balance_ > 0) return;

	Stats::instance().inc(stat_keys::gempiles_destroyed);

	Board::instance().removeObjectById(id_);
}
