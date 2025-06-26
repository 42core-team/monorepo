#ifndef ACTION_H
#define ACTION_H

#include <memory>
#include <vector>

#include "Config.h"
#include "Core.h"

class CreateAction;
class MoveAction;
class TransferMoneyAction;
class BuildAction;
class AttackAction;

#include "json.hpp"
using json = nlohmann::ordered_json;

enum class ActionType
{
	MOVE,
	CREATE,
	TRANSFER_MONEY,
	BUILD
};

class Action
{
public:
	Action(ActionType type);
	virtual ~Action() {}

	bool isValid() const { return is_valid_; }
	ActionType getActionType() const { return type_; }

	static std::vector<std::unique_ptr<Action>> parseActions(json msg);

	virtual bool execute(Core *core) = 0;
	virtual void decodeJSON(json msg) = 0;
	virtual json encodeJSON() = 0;

protected:
	bool is_valid_;

	ActionType type_;
};

#include "CreateAction.h"
#include "MoveAction.h"
#include "TransferMoneyAction.h"
#include "BuildAction.h"
#include "AttackAction.h"

#endif // ACTION_H
