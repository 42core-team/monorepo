#ifndef TRANSFER_MONEY_ACTION_H
#define TRANSFER_MONEY_ACTION_H

#include <vector>

#include "Action.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class TransferMoneyAction : public Action
{
	public:
		TransferMoneyAction(json msg);

		unsigned int getSourceObjId() const { return source_id_; }
		unsigned int getTargetObjId() const { return target_id_; }
		unsigned int getAmount() const { return amount_; }
	
	private:
		unsigned int source_id_;
		unsigned int target_id_;
		unsigned int amount_;
};

#endif // TRANSFER_MONEY_ACTION_H
