#include "AttackAction.h"

#include "Unit.h"

AttackAction::AttackAction(json msg) : Action(ActionType::ATTACK)
{
	decodeJSON(msg);
}

void AttackAction::decodeJSON(json msg)
{
	if (!msg.contains("unit_id") || !msg.contains("x") || !msg.contains("y"))
	{
		is_valid_ = false;
		return;
	}

	unit_id_ = msg["unit_id"];
	target_pos_.x = msg["x"];
	target_pos_.y = msg["y"];

	if (!target_pos_.isValid(Config::game().gridSize))
	{
		is_valid_ = false;
		return;
	}
}
json AttackAction::encodeJSON()
{
	json js;

	js["type"] = "attack";
	js["unit_id"] = unit_id_;
	js["x"] = target_pos_.x;
	js["y"] = target_pos_.y;

	return js;
}

std::string AttackAction::execute(Core *core)
{
	if (!is_valid_)
		return "invalid input";

	Object *unitObj = Board::instance().getObjectById(getUnitId());

	if (!unitObj || unitObj->getType() != ObjectType::Unit)
		return "invalid or non-existing unit";
	Unit *unit = (Unit *)unitObj;
	if (Board::instance().getObjectPositionById(unit->getId()).distance(target_pos_) > 1)
		return "unit is too far away";
	if (unit->getActionCooldown() > 0)
		return "unit is on cooldown";
	if (unit->getTeamId() != core->getTeamId())
		return "unit does not belong to your team";

	Object *obj = Board::instance().getObjectAtPos(target_pos_);
	if (!obj)
		return "no object at target position";

	unit->resetActionCooldown();

	// calculate attack damage depending on object type
	unsigned int damage = 1;
	if (obj->getType() == ObjectType::Unit)
	{
		damage = Config::game().units[unit->getUnitType()].damageUnit;
		obj->setHP(obj->getHP() - damage);

		Stats::instance().inc(stat_keys::damage_units, damage);
		if (((Unit *)obj)->getTeamId() != unit->getTeamId())
			Stats::instance().inc(stat_keys::damage_opponent, damage);
		else
			Stats::instance().inc(stat_keys::damage_self, damage);
	}
	else if (obj->getType() == ObjectType::Core)
	{
		damage = Config::game().units[unit->getUnitType()].damageCore;
		obj->setHP(obj->getHP() - damage);

		Stats::instance().inc(stat_keys::damage_cores, damage);
		if (((Core *)obj)->getTeamId() != unit->getTeamId())
			Stats::instance().inc(stat_keys::damage_opponent, damage);
		else
			Stats::instance().inc(stat_keys::damage_self, damage);
	}
	else if (obj->getType() == ObjectType::Deposit)
	{
		damage = Config::game().units[unit->getUnitType()].damageDeposit;
		obj->setHP(obj->getHP() - damage);
		if (obj->getHP() <= 0)
		{
			unsigned int gems = ((Deposit *)obj)->getBalance();
			Board::instance().removeObjectById(obj->getId());
			Board::instance().addObject<GemPile>(GemPile(gems), target_pos_);

			Stats::instance().inc(stat_keys::deposits_destroyed, 1);
		}

		Stats::instance().inc(stat_keys::damage_deposits, damage);
	}
	else if (obj->getType() == ObjectType::Wall)
	{
		damage = Config::game().units[unit->getUnitType()].damageWall;
		obj->setHP(obj->getHP() - damage);

		Stats::instance().inc(stat_keys::damage_walls, damage);
	}
	else if (obj->getType() == ObjectType::Bomb)
	{
		Bomb *bomb = (Bomb *)obj;
		bomb->handleAttack();
	}
	else if (obj->getType() == ObjectType::GemPile)
	{
		unsigned int gems = static_cast<GemPile *>(obj)->getBalance();
		unit->setBalance(unit->getBalance() + gems);
		Board::instance().removeObjectById(obj->getId());

		Stats::instance().inc(stat_keys::gems_gained, gems);
		Stats::instance().inc(stat_keys::gempiles_destroyed, 1);
	}

	Stats::instance().inc(stat_keys::actions_executed);
	Stats::instance().inc(stat_keys::damage_total, damage);

	return "";
}
