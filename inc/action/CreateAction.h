#ifndef CREATE_ACTION_H
#define CREATE_ACTION_H

#include <vector>

#include "Action.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class CreateAction : public Action
{
	public:
		CreateAction(json msg);

		unsigned int getUnitTypeId() const { return type_id_; }

		bool execute(Game *game, Core * core);
		void decodeJSON(json msg);
		json encodeJSON();
	
	private:
		unsigned int type_id_;
};

#endif // CREATE_ACTION_H
