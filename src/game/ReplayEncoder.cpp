#include "ReplayEncoder.h"

json ReplayEncoder::diffObject(const json& currentObj, const json& previousObj)
{
	json diff;
	diff["id"] = currentObj["id"];
	bool diffFound = false;

	for (auto it = currentObj.begin(); it != currentObj.end(); ++it)
	{
		const std::string& key = it.key();
		if (key == "id" || key == "nextMoveOpp")
			continue;

		if (previousObj.find(key) == previousObj.end() || previousObj.at(key) != it.value())
		{
			diff[key] = it.value();
			diffFound = true;
		}
	}
	return diffFound ? diff : json::object();
}

json ReplayEncoder::diffObjects(const json& currentObjects)
{
	json diffArray = json::array();

	std::unordered_map<int, json> currentMap;
	for (const auto& obj : currentObjects)
	{
		int id = obj["id"];
		currentMap[id] = obj;
	}

	for (const auto& [id, currentObj] : currentMap)
	{
		auto it = previousObjects_.find(id);
		if (it == previousObjects_.end())
		{
			diffArray.push_back(currentObj);
		}
		else
		{
			json objDiff = diffObject(currentObj, it->second);
			if (objDiff.size() > 1)
				diffArray.push_back(objDiff);
		}
	}

	for (const auto& [id, prevObj] : previousObjects_)
	{
		if (currentMap.find(id) == currentMap.end())
		{
			json deadObj;
			deadObj["id"] = id;
			deadObj["state"] = "dead";
			diffArray.push_back(deadObj);
		}
	}

	return diffArray;
}

void ReplayEncoder::addTickState(const json& state)
{
	unsigned long long tick = state.value("tick", 0);

	json currentObjects = state.value("objects", json::array());
	json objectsDiff;
	if (previousObjects_.empty())
		objectsDiff = currentObjects;
	else
		objectsDiff = diffObjects(currentObjects);

	previousObjects_.clear();
	for (const auto& obj : currentObjects)
	{
		int id = obj["id"];
		previousObjects_[id] = obj;
	}

	json actions = state.value("actions", json::array());

	json tickDiff;
	if (!objectsDiff.empty())
		tickDiff["objects"] = objectsDiff;
	if (!actions.empty())
		tickDiff["actions"] = actions;

	ticks_[std::to_string(tick)] = tickDiff;

	lastTickCount_ = tick;
}

void ReplayEncoder::includeConfig(json& config)
{
	config_ = config;
}

json ReplayEncoder::getReplay() const
{
	json finalReplay;
	finalReplay["ticks"] = ticks_;
	finalReplay["config"] = config_;
	finalReplay["full_tick_amount"] = lastTickCount_;
	return finalReplay;
}
