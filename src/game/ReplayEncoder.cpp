#include "ReplayEncoder.h"
#include <cstdlib>

#define REPLAY_VERSION std::string("1.0.0")

json ReplayEncoder::diffObject(const json &currentObj, const json &previousObj)
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
	if (!tickDiff.empty())
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

void ReplayEncoder::verifyReplaySaveFolder()
{
	for (const std::string &replaySaveFolder : Config::server().replayFolderPaths)
	{
		if (replaySaveFolder.empty())
		{
			Logger::Log(LogLevel::ERROR, "Replay save folder is not set.");
			std::exit(1);
		}

		if (replaySaveFolder.rfind("http://", 0) == 0 ||
			replaySaveFolder.rfind("https://", 0) == 0)
			return;

		if (!std::filesystem::exists(replaySaveFolder) ||
			!std::filesystem::is_directory(replaySaveFolder))
		{
			Logger::Log(LogLevel::ERROR, "Replay save folder is incorrectly set to: " + replaySaveFolder);
			exit(1);
		}
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
	miscSection["game_end_reason"] = static_cast<int>(gameEndReason_);
	miscSection["version"] = REPLAY_VERSION;

	const char *gameId = std::getenv("GAME_ID");
	miscSection["game_id"] = gameId ? std::string(gameId) : "";

	return miscSection;
}

void ReplayEncoder::exportReplay() const
{
	if (Config::server().replayFolderPaths.empty())
	{
		Logger::Log(LogLevel::ERROR, "No Replay save folder set. Cannot save replay.");
		return;
	}

	json replayData;
	replayData["misc"] = encodeMiscSection();
	replayData["ticks"] = ticks_;
	replayData["config"] = config_;
	replayData["full_tick_amount"] = lastTickCount_;

	saveReplay(replayData);
}
void ReplayEncoder::saveReplay(const json &replayData) const
{
	// std::string filePath = replaySaveFolder_ + "/replay_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json";
	// std::ofstream outFile(filePath);
	// if (!outFile.is_open())
	// {
	// 	Logger::Log(LogLevel::ERROR, "Could not open replay file for writing: " + filePath);
	// 	return;
	// }

	// outFile << replayData.dump();
	// outFile.close();

	for (const std::string &replaySaveFolder : Config::server().replayFolderPaths)
	{
		std::string latestPath = replaySaveFolder + "/replay_latest.json";
		std::ofstream outFile = std::ofstream(latestPath);
		if (!outFile.is_open())
		{
			Logger::Log(LogLevel::ERROR, "Could not open latest replay file for writing: " + latestPath);
			return;
		}
		outFile << replayData.dump();
		outFile.close();

		Logger::Log("Replay saved to " + latestPath + ".");
	}
}
