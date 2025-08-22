#include "Resource.h"

Resource::Resource()
	: Object(Config::game().resourceHp, ObjectType::Resource),
	  balance_(Config::game().resourceIncome) {}
Resource::Resource(unsigned int balance)
	: Object(Config::game().resourceHp,
			 ObjectType::Resource),
	  balance_(balance) {}

void Resource::tick(unsigned long long tickCount)
{
	(void)tickCount;
}
