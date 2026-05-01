#ifndef CREATE_ACTION_H
#define CREATE_ACTION_H

#include "Action.h"
#include "Utils.h"
#include "json.hpp"

#include <vector>
using json = nlohmann::ordered_json;

class CreateAction : public Action
{
  public:
	CreateAction(json msg);

	std::string execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

  private:
	std::vector<std::string> components_;
};

#endif // CREATE_ACTION_H
