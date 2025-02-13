#ifndef BUILD_ACTION_H
#define BUILD_ACTION_H

#include <vector>

#include "Action.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class BuildAction : public Action
{
	public:
		BuildAction(json msg);

		void execute(Game *game, Core * core);

		unsigned int getBuilderId() const { return builder_id_; }
		Position getPosition() const { return position_; }
	
	private:
		unsigned int builder_id_;
		Position position_;
};

#endif // BUILD_ACTION_H
