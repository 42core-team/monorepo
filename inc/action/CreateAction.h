#ifndef CREATE_ACTION_H
#define CREATE_ACTION_H

#include <vector>

#include "Action.h"
#include "Utils.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class CreateAction : public Action
{
public:
	CreateAction(json msg);

	unsigned int getUnitType() const { return unit_type_; }

	bool execute(Core *core);
	void decodeJSON(json msg);
	json encodeJSON();

private:
	unsigned int unit_type_;
};

#endif // CREATE_ACTION_H
