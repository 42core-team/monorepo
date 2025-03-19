#ifndef TRANSFER_MONEY_ACTION_H
#define TRANSFER_MONEY_ACTION_H

#include <vector>

#include "Action.h"
#include "Common.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class TransferMoneyAction : public Action
{
	public:
		TransferMoneyAction(json msg);

		bool execute(Game *game, Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int source_id_;
		Position target_;
		unsigned int amount_;

		bool dropMoney(Game *game, Core * core, Object *srcObj);
};

#endif // TRANSFER_MONEY_ACTION_H
