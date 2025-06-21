#ifndef MOVE_ACTION_H
#define MOVE_ACTION_H

#include <vector>

#include "Action.h"
#include "Common.h"
#include "Money.h"

class Unit;

#include "json.hpp"
using json = nlohmann::ordered_json;

class MoveAction : public Action
{
	public:
		MoveAction(json msg);

		unsigned int getUnitId() const { return unit_id_; }
		Position getTarget() const { return target_; }

		bool execute(Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int unit_id_;
		Position target_;
};

#endif // MOVE_ACTION_H
