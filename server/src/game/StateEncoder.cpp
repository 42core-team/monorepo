#include "StateEncoder.h"

#include "Config.h"

json StateEncoder::encodeFullState()
{
	json state = json::array();

	for (const Object &obj : Board::instance())
	{
		json o;

		o["id"] = obj.getId();
		o["type"] = (int)obj.getType();
		o["x"] = Board::instance().getObjectPositionById(obj.getId()).x;
		o["y"] = Board::instance().getObjectPositionById(obj.getId()).y;
		o["hp"] = obj.getHP();

		if (obj.getType() == ObjectType::Core)
		{
			o["teamId"] = ((Core &)obj).getTeamId();
			o["gems"] = ((Core &)obj).getBalance();
			o["SpawnCooldown"] = ((Core &)obj).getSpawnCooldown();
		}
		if (obj.getType() == ObjectType::Unit)
		{
			o["teamId"] = ((Unit &)obj).getTeamId();
			o["gems"] = ((Unit &)obj).getBalance();
			o["ActionCooldown"] = ((Unit &)obj).getActionCooldown();

			o["components"] = ((Unit &)obj).getComponents();
			o["properties"] = json::object();
			for (const auto &[prop, value] : ((Unit &)obj).getProperties())
				o["properties"][std::string(Config::unitPropertyToString(prop))] = value;
		}
		if (obj.getType() == ObjectType::Deposit)
		{
			o["gems"] = ((Deposit &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::GemPile)
		{
			o["gems"] = ((GemPile &)obj).getBalance();
		}

		if (obj.hasDebugInfo())
		{
			o["debug_info"] = obj.getDebugInfo();
		}
		if (obj.hasDebugPath())
		{
			json path = json::array();
			for (const auto &point : obj.getDebugPath())
			{
				json p;
				p["x"] = point.x;
				p["y"] = point.y;
				path.push_back(p);
			}
			o["debug_path"] = path;
		}

		state.push_back(o);
	}

	return state;
}

json StateEncoder::diffObject(const json &currentObj, const json &previousObj)
{
	json diff;
	diff["id"] = currentObj["id"];
	bool diffFound = false;

	for (auto it = currentObj.begin(); it != currentObj.end(); ++it)
	{
		const std::string &key = it.key();
		if (key == "id") continue;
		if (key == "ActionCooldown" && currentObj.contains("ActionCooldown") &&
			previousObj.contains("ActionCooldown") && currentObj["ActionCooldown"] <= previousObj["ActionCooldown"])
			continue;
		if (key == "SpawnCooldown" && currentObj.contains("SpawnCooldown") && previousObj.contains("SpawnCooldown") &&
			currentObj["SpawnCooldown"] <= previousObj["SpawnCooldown"])
			continue;

		if (previousObj.find(key) == previousObj.end() || previousObj.at(key) != it.value())
		{
			diff[key] = it.value();
			diffFound = true;
		}
	}
	return diffFound ? diff : json::object();
}

json StateEncoder::diffObjects(const json &previousObjects, const json &currentObjects)
{
	json diffArray = json::array();

	for (const auto &prevObj : previousObjects)
	{
		bool found = false;
		for (const auto &currentObj : currentObjects)
		{
			if (prevObj["id"] == currentObj["id"])
			{
				json objDiff = diffObject(currentObj, prevObj);
				if (!objDiff.empty()) diffArray.push_back(objDiff);
				found = true;
				break;
			}
		}
		if (!found)
		{
			json deadObj;
			deadObj["id"] = prevObj["id"];
			deadObj["state"] = "dead";
			diffArray.push_back(deadObj);
		}
	}
	for (const auto &currentObj : currentObjects)
	{
		bool existed = false;
		for (const auto &prevObj : previousObjects)
			if (prevObj["id"] == currentObj["id"])
			{
				existed = true;
				break;
			}
		if (!existed) diffArray.push_back(currentObj);
	}

	return diffArray;
}

json StateEncoder::generateObjectDiff()
{
	json returnJson = json::object();

	json fullState = encodeFullState();
	if (firstTick_)
		returnJson["objects"] = fullState;
	else
		returnJson["objects"] = diffObjects(previousObjects_, fullState);
	firstTick_ = false;

	previousObjects_ = fullState;

	return returnJson;
}
