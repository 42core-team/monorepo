#include "StateEncoder.h"

json StateEncoder::encodeFullState()
{
	json state = json::array();

	for (const Object & obj : Board::instance())
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
			o["balance"] = ((Core &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Unit)
		{
			o["teamId"] = ((Unit &)obj).getTeamId();
			o["unit_type"] = ((Unit &)obj).getUnitType();
			o["balance"] = ((Unit &)obj).getBalance();
			o["moveCooldown"] = ((Unit &)obj).getMoveCooldown();
		}
		if (obj.getType() == ObjectType::Resource)
		{
			o["balance"] = ((Resource &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Money)
		{
			o["balance"] = ((Money &)obj).getBalance();
		}
		if (obj.getType() == ObjectType::Bomb)
		{
			o["countdown"] = ((Bomb &)obj).getCountdown();
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
		if (key == "id" || (key == "moveCooldown" && currentObj.contains("moveCooldown") && previousObj.contains("moveCooldown") && currentObj["moveCooldown"] <= previousObj["moveCooldown"]))
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
				if (!objDiff.empty())
					diffArray.push_back(objDiff);
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
			if (prevObj["id"] == currentObj["id"]) { existed = true; break; }
		if (!existed)
			diffArray.push_back(currentObj);
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
