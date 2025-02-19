#ifndef MOVE_ACTION_H
#define MOVE_ACTION_H

#include <vector>

#include "Action.h"
#include "Common.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class MoveAction : public Action
{
	public:
		MoveAction(json msg);

		unsigned int getUnitId() const { return unit_id_; }
		MovementDirection getDirection() const { return dir_; }

		bool execute(Game *game, Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int unit_id_;
		MovementDirection dir_;
};

#endif // MOVE_ACTION_H
