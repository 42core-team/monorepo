#ifndef ACTION_H
#define ACTION_H

#include "json.hpp"
using json = nlohmann::json;

enum class ActionType {
    Create,
    Attack,
    Travel
};

struct Action {
    ActionType type;
    json data;

    static Action fromJson(const json& j) {
        Action action;
        std::string typeStr = j["type"];
        if (typeStr == "Create") {
            action.type = ActionType::Create;
        } else if (typeStr == "Attack") {
            action.type = ActionType::Attack;
        } else if (typeStr == "Travel") {
            action.type = ActionType::Travel;
        }
        action.data = j["data"];
        return action;
    }
    
    json toJson() const {
        json j;
        switch (type) {
            case ActionType::Create: j["type"] = "Create"; break;
            case ActionType::Attack: j["type"] = "Attack"; break;
            case ActionType::Travel: j["type"] = "Travel"; break;
        }
        j["data"] = data;
        return j;
    }
};

#endif // ACTION_H
