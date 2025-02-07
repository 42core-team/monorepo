#ifndef ACTION_H
#define ACTION_H

#include <vector>

#include "Config.h"
#include "Game.h"
class CreateAction;
class MoveAction;

#include "json.hpp"
using json = nlohmann::json;

enum class ActionType
{
	MOVE,
	CREATE
};

class Action
{
	public:
		Action(ActionType type);
		virtual ~Action() {}

		bool isValid() const { return is_valid_; }
		ActionType getActionType() const { return type_; }

		static std::vector<Action *> parseActions(json msg);

	protected:
		bool is_valid_;

		ActionType type_;
};

#include "CreateAction.h"
#include "MoveAction.h"

#endif // ACTION_H

