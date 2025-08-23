#include "Deposit.h"

Deposit::Deposit()
	: Object(Config::game().depositHp, ObjectType::Deposit),
	  balance_(Config::game().depositIncome) {}
Deposit::Deposit(unsigned int gems)
	: Object(Config::game().depositHp,
			 ObjectType::Deposit),
	  balance_(gems) {}

void Deposit::tick(unsigned long long tickCount)
{
	(void)tickCount;
}
