#include "TransferGemsAction.h"

TransferGemsAction::TransferGemsAction(json msg) : Action(ActionType::TRANSFER_GEMS)
{
	decodeJSON(msg);
}

void TransferGemsAction::decodeJSON(json msg)
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
json TransferGemsAction::encodeJSON()
{
	json js;

	js["type"] = "transfer_gems";
	js["source_id"] = source_id_;
	js["x"] = target_.x;
	js["y"] = target_.y;
	js["amount"] = amount_;

	return js;
}

std::string TransferGemsAction::dropGems(Core *core, Object *srcObj)
{
	if (!target_.isValid(Config::game().gridSize)) return "target position is out of bounds";

	if (srcObj->getType() != ObjectType::Unit) return "only units can drop gems on the floor";

	if (Board::instance().getObjectPositionById(srcObj->getId()).distance(target_) > 1) return "invalid drop position";

	if (Board::instance().getObjectPositionById(srcObj->getId()) == target_) return "invalid drop position";

	Unit *srcUnit = (Unit *)srcObj;
	if (srcUnit->getTeamId() != core->getTeamId()) return "can't drop gems from another team";
	if (srcUnit->getActionCooldown() > 0) return "unit is on action cooldown (action cooldown should be 0)";

	if (srcUnit->getBalance() < amount_) amount_ = srcUnit->getBalance();
	if (amount_ <= 0) return "invalid amount";
	srcUnit->setBalance(srcUnit->getBalance() - amount_);

	srcUnit->resetActionCooldown();

	Board::instance().addObject<GemPile>(GemPile(amount_), target_);

	Stats::instance().inc(stat_keys::actions_executed);
	Stats::instance().inc(stat_keys::gems_transferred, amount_);

	return "";
}

std::string TransferGemsAction::execute(Core *core)
{
	if (!is_valid_) return "invalid input";

	Object *srcObj = Board::instance().getObjectById(source_id_);
	if (!srcObj) return "no object with source_id " + std::to_string(source_id_);

	Object *dstObj = Board::instance().getObjectAtPos(target_);
	if (!dstObj) return dropGems(core, srcObj);

	if (srcObj->getId() == dstObj->getId()) return "can't transfer gems to yourself"; // can't transfer gems to itself

	// only active objects can transfer gems
	if (srcObj->getType() != ObjectType::Core && srcObj->getType() != ObjectType::Unit)
		return "invalid source object type";
	if (dstObj->getType() != ObjectType::Core && dstObj->getType() != ObjectType::Unit &&
		dstObj->getType() != ObjectType::GemPile)
		return "invalid destination object type. please transfer gems only ot object that can hold gems (e.g. cores, "
			   "units, gem piles).";

	// only as-close-together-as-possible objects can transfer gems
	Position srcPos = Board::instance().getObjectPositionById(srcObj->getId());
	Position dstPos = Board::instance().getObjectPositionById(dstObj->getId());
	Position firstEmptyGridCell = findFirstEmptyGridCell(dstPos);
	unsigned int maxDist = dstPos.distance(firstEmptyGridCell);
	if (srcPos.distance(dstPos) > maxDist)
		return "invalid transfer distance; objects aren't as close as possible in Manhattan distance";

	// cant transfer someone else's gems
	if (srcObj->getType() == ObjectType::Core)
	{
		Core *srcCore = (Core *)srcObj;
		if (srcCore->getTeamId() != core->getTeamId()) return "can't transfer gems from another team core";
		if (srcCore->getBalance() < amount_) amount_ = srcCore->getBalance();
		if (srcCore->getBalance() <= 0)
			return "invalid amount (tried to transfer " + std::to_string(amount_) + " gems)";
		srcCore->setBalance(srcCore->getBalance() - amount_);
	}
	if (srcObj->getType() == ObjectType::Unit)
	{
		Unit *srcUnit = (Unit *)srcObj;
		if (srcUnit->getTeamId() != core->getTeamId()) return "can't transfer gems from another team unit";
		if (srcUnit->getActionCooldown() > 0) return "unit is on action cooldown (action cooldown should be 0)";
		srcUnit->resetActionCooldown();
		if (srcUnit->getBalance() < amount_) amount_ = srcUnit->getBalance();
		if (srcUnit->getBalance() <= 0)
			return "invalid amount (tried to transfer " + std::to_string(amount_) + " gems)";
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
	if (dstObj->getType() == ObjectType::GemPile)
	{
		GemPile *dstGemPile = (GemPile *)dstObj;
		dstGemPile->setBalance(dstGemPile->getBalance() + amount_);
	}

	Stats::instance().inc(stat_keys::actions_executed);
	Stats::instance().inc(stat_keys::gems_transferred, amount_);

	return "";
}
