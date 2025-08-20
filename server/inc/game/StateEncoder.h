#ifndef STATE_ENCODER_H
#define STATE_ENCODER_H

#include "json.hpp"
using json = nlohmann::ordered_json;

#include "Action.h"
#include "Board.h"
#include "Core.h"

class StateEncoder
{
public:
	StateEncoder() = default;
	~StateEncoder() = default;

	json generateObjectDiff();

private:
	json encodeFullState();
	json diffObject(const json &currentObj, const json &previousObj);
	json diffObjects(const json &previousObjects, const json &currentObjects);

	json previousObjects_;
	bool firstTick_ = true;
};

#endif // STATE_ENCODER_H
