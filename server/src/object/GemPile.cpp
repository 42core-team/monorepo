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
