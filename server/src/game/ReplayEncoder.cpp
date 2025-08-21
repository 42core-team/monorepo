#include "ReplayEncoder.h"
#include <cstdlib>

#define REPLAY_VERSION std::string("1.1.1")

ReplayEncoder& ReplayEncoder::instance() { static ReplayEncoder inst; return inst; }

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

void ReplayEncoder::registerExpectedTeam(unsigned int teamId)
{
	if (!teamData_.count(teamId)) teamData_[teamId].teamId = teamId;
}
void ReplayEncoder::setTeamName(unsigned int teamId, const std::string& teamName)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].teamName = teamName;
}
void ReplayEncoder::markConnectedInitially(unsigned int teamId, bool connected)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].connectedInitially = connected;
}
void ReplayEncoder::setDeathReason(unsigned int teamId, death_reason_t reason)
{
	registerExpectedTeam(teamId);
	if (static_cast<int>(teamData_[teamId].deathReason) < 4) // dont overwrite timeout death reasons
		teamData_[teamId].deathReason = reason;
}
void ReplayEncoder::setPlace(unsigned int teamId, unsigned int place)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].place = place;
}

bool ReplayEncoder::wasConnectedInitially(unsigned int teamId) const {
	auto it = teamData_.find(teamId);
	return it != teamData_.end() && it->second.connectedInitially;
}

void ReplayEncoder::includeConfig(json &config)
{
	config_ = config;
}

void ReplayEncoder::verifyReplaySaveFolder()
{
	std::vector<std::string> validReplaySaveFolders;
	for (const std::string &replaySaveFolder : Config::server().replayFolderPaths)
	{
		if (replaySaveFolder.empty())
		{
			Logger::Log(LogLevel::ERROR, "Replay save folder is not set.");
			continue;
		}

		if (!std::filesystem::exists(replaySaveFolder) ||
			!std::filesystem::is_directory(replaySaveFolder))
		{
			Logger::Log(LogLevel::ERROR, "One replay save folder is incorrectly set to: " + replaySaveFolder);
			continue;
		}

		validReplaySaveFolders.push_back(replaySaveFolder);
	}

	Config::server().replayFolderPaths = validReplaySaveFolders;

	if (validReplaySaveFolders.empty())
	{
		Logger::Log(LogLevel::ERROR, "No valid replay save folders found.");
		exit(1);
	}
}

json ReplayEncoder::encodeMiscSection() const
{
	json miscSection;

	json players = json::array();
	for (const auto& kv : teamData_)
	{
		const auto& p = kv.second;
		json pj;
		pj["id"] = p.teamId;
		pj["name"] = p.teamName;
		pj["place"] = p.place;
		if (p.connectedInitially)
			pj["death_reason"] = static_cast<int>(p.deathReason);
		else
			pj["death_reason"] = static_cast<int>(death_reason_t::DID_NOT_CONNECT);
		players.push_back(pj);
	}
	miscSection["team_results"] = players;

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
	replayData["ticks"] = !ticks_.empty() ? ticks_ : json::array();
	replayData["config"] = config_;
	replayData["full_tick_amount"] = lastTickCount_;

	saveReplay(replayData);
}
void ReplayEncoder::saveReplay(const json &replayData) const
{
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
