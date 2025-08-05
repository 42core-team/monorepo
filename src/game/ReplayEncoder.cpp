#include "ReplayEncoder.h"
#include <cstdlib>

#define REPLAY_VERSION std::string("1.0.1")

void ReplayEncoder::addTickState(json &state, unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions)
{
	if (actions.size() > 0)
	{
		state["actions"] = json::array();
		for (const auto &action : actions)
			if (action.first)
				state["actions"].push_back(action.first.get()->encodeJSON());
	}

	if (!state.empty())
		ticks_[std::to_string(tick)] = state;

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
	for (auto& kv : customData_.items())
		replayData["misc"][kv.key()] = kv.value();
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
