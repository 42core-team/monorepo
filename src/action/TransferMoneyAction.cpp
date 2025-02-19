#include "TransferMoneyAction.h"

TransferMoneyAction::TransferMoneyAction(json msg) : Action(ActionType::TRANSFER_MONEY)
{
	decodeJSON(msg);
}

void TransferMoneyAction::decodeJSON(json msg)
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
json TransferMoneyAction::encodeJSON()
{
	json js;

	js["type"] = "transfer_money";
	js["source_id"] = source_id_;
	js["target_id"] = target_id_;
	js["amount"] = amount_;

	return js;
}

bool TransferMoneyAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return false;

	Object * srcObj = game->getObject(getSourceObjId());
	Object * dstObj = game->getObject(getTargetObjId());
	if (!srcObj || !dstObj)
		return false;

	// only active objects can transfer money
	if (srcObj->getType() != ObjectType::Core && srcObj->getType() != ObjectType::Unit)
		return false;
	if (dstObj->getType() != ObjectType::Core && dstObj->getType() != ObjectType::Unit)
		return false;

	// only adjacent objects can transfer money
	if (srcObj->getPosition().distance(dstObj->getPosition()) > 1)
		return false;

	unsigned int amount = getAmount();

	// cant transfer someone else's money
	if (srcObj->getType() == ObjectType::Core)
	{
		Core * srcCore = (Core *)srcObj;
		if (srcCore->getTeamId() != core->getTeamId())
			return false;
		if (srcCore->getBalance() < amount)
			amount = srcCore->getBalance();
		srcCore->setBalance(srcCore->getBalance() - amount);
	}
	if (srcObj->getType() == ObjectType::Unit)
	{
		Unit * srcUnit = (Unit *)srcObj;
		if (srcUnit->getTeamId() != core->getTeamId())
			return false;
		if (srcUnit->getBalance() < amount)
			amount = srcUnit->getBalance();
		srcUnit->setBalance(srcUnit->getBalance() - amount);
	}

	if (dstObj->getType() == ObjectType::Core)
	{
		Core * dstCore = (Core *)dstObj;
		dstCore->setBalance(dstCore->getBalance() + amount);
	}
	if (dstObj->getType() == ObjectType::Unit)
	{
		Unit * dstUnit = (Unit *)dstObj;
		dstUnit->setBalance(dstUnit->getBalance() + amount);
	}

	return true;
}
