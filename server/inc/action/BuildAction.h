#ifndef BUILD_ACTION_H
#define BUILD_ACTION_H

#include "Action.h"
#include "Wall.h"
#include "json.hpp"

#include <vector>
using json = nlohmann::ordered_json;

class BuildAction : public Action
{
  public:
	BuildAction(json msg);

	std::string execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

	unsigned int getBuilderId() const { return builder_id_; }
	Position getPosition() const { return position_; }

  private:
	unsigned int builder_id_;
	Position position_;
};

#endif // BUILD_ACTION_H
