#include "TransferMoneyAction.h"

TransferMoneyAction::TransferMoneyAction(json msg) : Action(ActionType::TRANSFER_MONEY)
{
	decodeJSON(msg);
}

void TransferMoneyAction::decodeJSON(json msg)
{
	if (!msg.contains("source_id") || !msg.contains("x") || !msg.contains("y") || !msg.contains("amount"))
	{
		is_valid_ = false;
		return;
	}

	source_id_ = msg["source_id"];
	int y = msg["x"];
	int x = msg["y"];
	target_ = Position(x, y);
	amount_ = msg["amount"];
}
json TransferMoneyAction::encodeJSON()
{
	json js;

	js["type"] = "transfer_money";
	js["source_id"] = source_id_;
	js["x"] = target_.x;
	js["y"] = target_.y;
	js["amount"] = amount_;

	return js;
}

bool TransferMoneyAction::dropMoney(Game *game, Core * core, Object *srcObj)
{
	if (srcObj->getType() != ObjectType::Unit)
		return false;

	if (srcObj->getPosition().distance(target_) > 1)
		return false;

	if (srcObj->getPosition() == target_)
		return false;

	Unit * srcUnit = (Unit *)srcObj;
	if (srcUnit->getTeamId() != core->getTeamId())
		return false;

	if (srcUnit->getBalance() < amount_)
		amount_ = srcUnit->getBalance();
	srcUnit->setBalance(srcUnit->getBalance() - amount_);

	Position pos = srcUnit->getPosition();
	game->getObjects().push_back(std::make_unique<Money>(game->getNextObjectId(), pos, amount_));

	return true;
}

bool TransferMoneyAction::execute(Game *game, Core * core)
{
	if (!is_valid_)
		return false;

	Object * srcObj = game->getObject(source_id_);
	if (!srcObj)
		return false;

	Object * dstObj = game->getObjectAtPos(target_);
	if (!dstObj)
		return dropMoney(game, core, srcObj);

	// only active objects can transfer money
	if (srcObj->getType() != ObjectType::Core && srcObj->getType() != ObjectType::Unit)
		return false;
	if (dstObj->getType() != ObjectType::Core && dstObj->getType() != ObjectType::Unit)
		return false;

	// only adjacent objects can transfer money
	if (srcObj->getPosition().distance(dstObj->getPosition()) > 1)
		return false;

	unsigned int amount = amount_;

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
