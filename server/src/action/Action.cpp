#include "Action.h"

#include "Logger.h"

#include <json-schema.hpp>
#include <unordered_map>
using nlohmann::json_schema::json_validator;

Action::Action(ActionType type) : type_(type)
{
}

static inline const char *schema_for_type(const std::string &t)
{
	if (t == "move") return "packets/actions/action-move.schema.json";
	if (t == "create") return "packets/actions/action-create.schema.json";
	if (t == "transfer_gems") return "packets/actions/action-transfer-gems.schema.json";
	if (t == "attack") return "packets/actions/action-attack.schema.json";
	return nullptr;
}

// compiled once per schema, a packet can hold many actions and rereading the schema for each is needlessly expensive
static const json_validator &validator_for_schema(const char *schemaName)
{
	static std::unordered_map<std::string, json_validator> validators;
	auto it = validators.find(schemaName);
	if (it == validators.end())
	{
		json_validator v;
		v.set_root_schema(Config::load_json_schema(schemaName));
		it = validators.emplace(schemaName, std::move(v)).first;
	}
	return it->second;
}

std::vector<std::unique_ptr<Action>> Action::parseActions(json msg, std::vector<std::string> *errorsOut)
{
	if (!msg.contains("actions")) return std::vector<std::unique_ptr<Action>>();

	std::vector<std::unique_ptr<Action>> actions;

	for (auto &actionJson : msg["actions"])
	{
		std::unique_ptr<Action> newAction;
		if (actionJson.contains("type") && actionJson["type"].is_string())
		{
			const std::string t = actionJson["type"];
			if (const char *schemaName = schema_for_type(t))
			{
				// Validate action schema
				try
				{
					validator_for_schema(schemaName).validate(actionJson);
				}
				catch (const std::exception &e)
				{
					Logger::Log(LogLevel::WARNING, std::string("Action schema validation failed for type ") +
														   json(t).dump() + ": " + e.what() + " (\"" +
														   actionJson.dump() + "\")");
					if (errorsOut)
						errorsOut->emplace_back(std::string("Action Failure: Parsing Problem: ") + t + ": " + e.what());
					continue;
				}

				if (t == "move")
					newAction = std::make_unique<MoveAction>(actionJson);
				else if (t == "create")
					newAction = std::make_unique<CreateAction>(actionJson);
				else if (t == "transfer_gems")
					newAction = std::make_unique<TransferGemsAction>(actionJson);
				else if (t == "attack")
					newAction = std::make_unique<AttackAction>(actionJson);
			}
			else
			{
				Logger::Log(LogLevel::WARNING, std::string("Unknown action type ") + json(t).dump() +
													   " – discarding. (\"" + actionJson.dump() + "\")");
				if (errorsOut)
				{
					errorsOut->emplace_back(std::string("Action Failure: Parsing Problem: unknown type '") + t + "' (" +
											actionJson.dump() + ")");
				}
			}
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
	default:
		return "unknown";
	}
}
