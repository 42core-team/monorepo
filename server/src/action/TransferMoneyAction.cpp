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
	int y = msg["y"];
	int x = msg["x"];
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

std::string TransferMoneyAction::dropMoney(Core *core, Object *srcObj)
{
	if (srcObj->getType() != ObjectType::Unit)
		return "only units can drop money on the floor";

	if (Board::instance().getObjectPositionById(srcObj->getId()).distance(target_) > 1)
		return "invalid drop position";

	if (Board::instance().getObjectPositionById(srcObj->getId()) == target_)
		return "invalid drop position";

	Unit *srcUnit = (Unit *)srcObj;
	if (srcUnit->getTeamId() != core->getTeamId())
		return "can't drop money from another team";
	if (srcUnit->getMoveCooldown() > 0)
		return "unit is on cooldown";

	if (srcUnit->getBalance() < amount_)
		amount_ = srcUnit->getBalance();
	if (amount_ <= 0)
		return "invalid amount";
	srcUnit->setBalance(srcUnit->getBalance() - amount_);

	srcUnit->resetMoveCooldown();

	Board::instance().addObject<Money>(Money(Board::instance().getNextObjectId(), amount_), target_);

	return "";
}

std::string TransferMoneyAction::execute(Core *core)
{
	if (!is_valid_)
		return "invalid input";

	Object *srcObj = Board::instance().getObjectById(source_id_);
	if (!srcObj)
		return "invalid source object";

	Object *dstObj = Board::instance().getObjectAtPos(target_);
	if (!dstObj)
		return dropMoney(core, srcObj);

	if (srcObj->getId() == dstObj->getId())
		return "can't transfer money to yourself"; // can't transfer money to itself

	// only active objects can transfer money
	if (srcObj->getType() != ObjectType::Core && srcObj->getType() != ObjectType::Unit)
		return "invalid source object type";
	if (dstObj->getType() != ObjectType::Core && dstObj->getType() != ObjectType::Unit)
		return "invalid destination object type";

	// only as-close-together-as-possible objects can transfer money
	Position srcPos = Board::instance().getObjectPositionById(srcObj->getId());
	Position dstPos = Board::instance().getObjectPositionById(dstObj->getId());
	Position firstEmptyGridCell = findFirstEmptyGridCell(dstPos);
	unsigned int maxDist = dstPos.distance(firstEmptyGridCell);
	if (srcPos.distance(dstPos) > maxDist)
		return "invalid transfer distance";

	// cant transfer someone else's money
	if (srcObj->getType() == ObjectType::Core)
	{
		Core *srcCore = (Core *)srcObj;
		if (srcCore->getTeamId() != core->getTeamId())
			return "can't transfer money from another team core";
		if (srcCore->getBalance() < amount_)
			amount_ = srcCore->getBalance();
		if (srcCore->getBalance() <= 0)
			return "invalid amount";
		srcCore->setBalance(srcCore->getBalance() - amount_);
	}
	if (srcObj->getType() == ObjectType::Unit)
	{
		Unit *srcUnit = (Unit *)srcObj;
		if (srcUnit->getTeamId() != core->getTeamId())
			return "can't transfer money from another team unit";
		if (srcUnit->getMoveCooldown() > 0)
			return "unit is on cooldown";
		srcUnit->resetMoveCooldown();
		if (srcUnit->getBalance() < amount_)
			amount_ = srcUnit->getBalance();
		if (srcUnit->getBalance() <= 0)
			return "invalid amount";
		srcUnit->setBalance(srcUnit->getBalance() - amount_);
	}

	if (dstObj->getType() == ObjectType::Core)
	{
		Core *dstCore = (Core *)dstObj;
		dstCore->setBalance(dstCore->getBalance() + amount_);
	}
	if (dstObj->getType() == ObjectType::Unit)
	{
		Unit *dstUnit = (Unit *)dstObj;
		dstUnit->setBalance(dstUnit->getBalance() + amount_);
	}

	return "";
}
