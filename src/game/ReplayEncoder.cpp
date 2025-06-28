#include "ReplayEncoder.h"

std::string ReplayEncoder::replaySaveFolder_ = "";

json ReplayEncoder::diffObject(const json &currentObj, const json &previousObj)
{
	json diff;
	diff["id"] = currentObj["id"];
	bool diffFound = false;

	for (auto it = currentObj.begin(); it != currentObj.end(); ++it)
	{
		const std::string &key = it.key();
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

json ReplayEncoder::diffObjects(const json &currentObjects)
{
	json diffArray = json::array();

	std::unordered_map<int, json> currentMap;
	for (const auto &obj : currentObjects)
	{
		int id = obj["id"];
		currentMap[id] = obj;
	}

	for (const auto &[id, currentObj] : currentMap)
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

	for (const auto &[id, prevObj] : previousObjects_)
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

void ReplayEncoder::addTickState(const json &state)
{
	unsigned long long tick = state.value("tick", 0);

	json currentObjects = state.value("objects", json::array());
	json objectsDiff;
	if (previousObjects_.empty())
		objectsDiff = currentObjects;
	else
		objectsDiff = diffObjects(currentObjects);

	previousObjects_.clear();
	for (const auto &obj : currentObjects)
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

void ReplayEncoder::addTeamScore(unsigned int teamId, const std::string &teamName, unsigned int place)
{
	team_data_t teamData;
	teamData.teamId = teamId;
	teamData.teamName = teamName;
	teamData.place = place;

	teamData_.push_back(teamData);
}

void ReplayEncoder::includeConfig(json &config)
{
	config_ = config;
}

void ReplayEncoder::setReplaySaveFolder(const std::string &folder)
{
	std::string trimmedFolder = folder;
	while (!trimmedFolder.empty() && trimmedFolder.back() == '/')
		trimmedFolder.pop_back();
	replaySaveFolder_ = trimmedFolder;
}
void ReplayEncoder::verifyReplaySaveFolder()
{
	if (replaySaveFolder_.empty() ||
		!std::filesystem::exists(replaySaveFolder_) ||
		!std::filesystem::is_directory(replaySaveFolder_))
	{
		Logger::Log(LogLevel::ERROR, "Replay save folder is incorrectly set to: " + replaySaveFolder_);
		exit(1);
	}
}

json ReplayEncoder::encodeMiscSection() const
{
	json miscSection;
	miscSection["team_results"] = json::array();
	for (const auto &team : teamData_)
	{
		json teamJson;
		teamJson["id"] = team.teamId;
		teamJson["name"] = team.teamName;
		teamJson["place"] = team.place;
		miscSection["team_results"].push_back(teamJson);
	}
	return miscSection;
}

void ReplayEncoder::saveReplay() const
{
	if (replaySaveFolder_.empty())
	{
		Logger::Log(LogLevel::ERROR, "Replay save folder is not set. Cannot save replay.");
		return;
	}

	json replayData;
	replayData["misc"] = encodeMiscSection();
	replayData["ticks"] = ticks_;
	replayData["config"] = config_;
	replayData["full_tick_amount"] = lastTickCount_;

	std::string filePath = replaySaveFolder_ + "/replay_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json";
	std::ofstream outFile(filePath);
	if (!outFile.is_open())
	{
		Logger::Log(LogLevel::ERROR, "Could not open replay file for writing: " + filePath);
		return;
	}

	outFile << replayData.dump();
	outFile.close();

	std::string latestPath = replaySaveFolder_ + "/replay_latest.json";
	outFile = std::ofstream(latestPath);
	if (!outFile.is_open())
	{
		Logger::Log(LogLevel::ERROR, "Could not open latest replay file for writing: " + latestPath);
		return;
	}
	outFile << replayData.dump();
	outFile.close();

	Logger::Log("Replay saved to " + filePath + " and latest replay updated.");
}
