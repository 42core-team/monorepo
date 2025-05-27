#ifndef ATTACK_ACTION_H
#define ATTACK_ACTION_H

#include <vector>

#include "Action.h"
#include "Common.h"
#include "Money.h"

class Unit;

#include "json.hpp"
using json = nlohmann::ordered_json;

class AttackAction : public Action
{
	public:
		AttackAction(json msg);
		unsigned int getUnitId() const { return unit_id_; }
		MovementDirection getDirection() const { return dir_; }
		unsigned int getTargetId() const { return target_id_; }
		unsigned int getDamage() const { return damage_; }

		bool execute(Game *game, Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int unit_id_;
		MovementDirection dir_;
		unsigned int target_id_;
		unsigned int damage_;

		bool attackObj(Object *obj, Unit * unit, Game *game);
};

#endif // ATTACK_ACTION_H
