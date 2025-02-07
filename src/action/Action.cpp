#include "Action.h"

Action::Action(ActionType type) : is_valid_(true), type_(type) { }

std::vector<Action *> Action::parseActions(json msg)
{
	if (!msg.contains("actions"))
		return std::vector<Action *>();

	std::vector<Action *> actions;

	for (auto & actionJson : msg["actions"])
	{
		Action * newAction = nullptr;
		if (actionJson.contains("type"))
		{
			if (actionJson["type"] == "move")
				newAction = new MoveAction(actionJson);
			else if (actionJson["type"] == "create")
				newAction = new CreateAction(actionJson);
			if (newAction && !newAction->is_valid_)
			{
				delete newAction;
				newAction = nullptr;
			}
		}

		if (newAction != nullptr)
			actions.push_back(newAction);
	}

	return actions;
}
