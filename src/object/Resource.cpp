#include "Resource.h"

Resource::Resource(unsigned int id)
	: Object(id, Config::game().resourceHp, ObjectType::Resource),
	  balance_(Config::game().resourceIncome) {}
Resource::Resource(unsigned int id, unsigned int balance)
	: Object(id, Config::game().resourceHp,
			 ObjectType::Resource),
	  balance_(balance) {}

void Resource::tick(unsigned long long tickCount)
{
	(void)tickCount;
}
