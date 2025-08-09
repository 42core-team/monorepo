#ifndef ATTACK_ACTION_H
#define ATTACK_ACTION_H

#include <vector>

#include "Action.h"
#include "Common.h"
#include "Money.h"
#include "Bomb.h"

#include "Resource.h"
#include "Board.h"

class Unit;

#include "json.hpp"
using json = nlohmann::ordered_json;

class AttackAction : public Action
{
	public:
		AttackAction(json msg);
		unsigned int getUnitId() const { return unit_id_; }
		Position getTargetPos() const { return target_pos_; }

		std::string execute(Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int unit_id_;
		Position target_pos_;

		std::string attackObj(Object *obj, Unit * unit);
};

#endif // ATTACK_ACTION_H
