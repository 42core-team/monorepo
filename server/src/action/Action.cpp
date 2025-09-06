#include "Action.h"

Action::Action(ActionType type) : is_valid_(true), type_(type)
{
}

std::vector<std::unique_ptr<Action>> Action::parseActions(json msg)
{
	if (!msg.contains("actions")) return std::vector<std::unique_ptr<Action>>();

	std::vector<std::unique_ptr<Action>> actions;

	for (auto &actionJson : msg["actions"])
	{
		std::unique_ptr<Action> newAction;
		if (actionJson.contains("type"))
		{
			if (actionJson["type"] == "move")
				newAction = std::make_unique<MoveAction>(actionJson);
			else if (actionJson["type"] == "create")
				newAction = std::make_unique<CreateAction>(actionJson);
			else if (actionJson["type"] == "transfer_gems")
				newAction = std::make_unique<TransferGemsAction>(actionJson);
			else if (actionJson["type"] == "build")
				newAction = std::make_unique<BuildAction>(actionJson);
			else if (actionJson["type"] == "attack")
				newAction = std::make_unique<AttackAction>(actionJson);
			if (newAction && !newAction->is_valid_) newAction = nullptr;
		}

		if (newAction != nullptr) actions.emplace_back(std::move(newAction));
	}

	return actions;
}

std::string Action::getActionName(ActionType type)
{
	switch (type)
	{
	case ActionType::MOVE:
		return "move";
	case ActionType::ATTACK:
		return "attack";
	case ActionType::CREATE:
		return "create";
	case ActionType::TRANSFER_GEMS:
		return "transfer_gems";
	case ActionType::BUILD:
		return "build";
	default:
		return "unknown";
	}
}
