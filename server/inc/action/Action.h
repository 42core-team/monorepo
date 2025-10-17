#ifndef ACTION_H
#define ACTION_H

#include "Config.h"
#include "Core.h"

#include <memory>
#include <vector>

class CreateAction;
class MoveAction;
class TransferGemsAction;
class BuildAction;
class AttackAction;

#include "json.hpp"
using json = nlohmann::ordered_json;

enum class ActionType
{
	MOVE,
	ATTACK,
	CREATE,
	TRANSFER_GEMS,
	BUILD
};

class Action
{
  public:
	Action(ActionType type);
	virtual ~Action() {}

	ActionType getActionType() const { return type_; }

	static std::vector<std::unique_ptr<Action>> parseActions(json msg, std::vector<std::string> *errorsOut = nullptr);
	static std::string getActionName(ActionType type);

	virtual std::string execute(Core *core) = 0;
	virtual void decodeJSON(json msg) = 0;
	virtual json encodeJSON() = 0;

  protected:
	ActionType type_;
};

#include "AttackAction.h"
#include "BuildAction.h"
#include "CreateAction.h"
#include "MoveAction.h"
#include "TransferGemsAction.h"

#endif // ACTION_H
