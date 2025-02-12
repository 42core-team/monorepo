#include "TransferMoneyAction.h"

TransferMoneyAction::TransferMoneyAction(json msg) : Action(ActionType::TRANSFER_MONEY)
{
	if (!msg.contains("source_id") || !msg.contains("target_id") || !msg.contains("amount"))
	{
		is_valid_ = false;
		return;
	}
	
	source_id_ = msg["source_id"];
	target_id_ = msg["target_id"];
	amount_ = msg["amount"];
}
