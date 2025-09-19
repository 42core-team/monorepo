#include "SetUnitDataAction.h"

#include <iostream>

#include "Unit.h"

class Unit;

SetUnitDataAction::SetUnitDataAction(json msg) : Action(ActionType::SET_UNIT_DATA) {
    decodeJSON(msg);
}

void SetUnitDataAction::decodeJSON(json msg) {
    if (!msg.contains("path")
        || !msg["path"].is_array()
        || !msg.contains("unit_id")
        || !msg["unit_id"].is_number_unsigned()) {
        is_valid_ = false;
        return;
    }

    unit_id_ = msg["unit_id"].get<unsigned int>();

    for (auto pos: msg["path"]) {
        if (!pos.is_object()
            || !pos.contains("x")
            || !pos.contains("y")
            || !pos["x"].is_number_integer()
            || !pos["y"].is_number_integer()) {
            is_valid_ = false;
            return;
        }
        int x = pos["x"];
        int y = pos["y"];
        Position p(x, y);

        if (!p.isValid(Config::game().gridSize)) {
            is_valid_ = false;
            return;
        }

        path_.push_back(p);
    }
}

json SetUnitDataAction::encodeJSON() {
    json js;

	js["type"] = "set_unit_data";

    for (auto pos: path_) {
        js["path"]["x"] = pos.x;
        js["path"]["y"] = pos.y;
    }

    return js;
}

std::string SetUnitDataAction::execute(Core *core) {
    if (!is_valid_) return "invalid input";

    Object *unitObj = Board::instance().getObjectById(unit_id_);
    if (!unitObj || unitObj->getType() != ObjectType::Unit) return "invalid or non-existing unit";
    Unit *unit = static_cast<Unit *>(unitObj);
    if (unit->getTeamId() != core->getTeamId()) return "unit does not belong to your team";

    unit->setDebugPath(path_);

    return "";
}
