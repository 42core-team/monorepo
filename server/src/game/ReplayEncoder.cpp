#include "ReplayEncoder.h"

#include "ReplayStream.h"

#include <array>
#include <iomanip>
#include <random>
#include <sstream>

#define REPLAY_VERSION std::string("3.0.0")

static std::string makeGameId()
{
	const char *provided = std::getenv("GAME_ID");
	if (provided && *provided) return provided;

	std::array<unsigned char, 16> bytes;
	std::random_device random;
	for (unsigned char &byte : bytes)
		byte = static_cast<unsigned char>(random());
	bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0f) | 0x40);
	bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3f) | 0x80);

	std::ostringstream id;
	id << "local-" << std::hex << std::setfill('0');
	for (size_t i = 0; i < bytes.size(); ++i)
	{
		id << std::setw(2) << static_cast<unsigned int>(bytes[i]);
		if (i == 3 || i == 5 || i == 7 || i == 9) id << '-';
	}
	return id.str();
}

ReplayEncoder::ReplayEncoder() : gameId_(makeGameId()), lastTickCount_(0)
{
}

ReplayEncoder &ReplayEncoder::instance()
{
	static ReplayEncoder inst;
	return inst;
}

void ReplayEncoder::addTickState(json &state, unsigned long long tick,
								 std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions)
{
	if (actions.size() > 0)
	{
		state["actions"] = json::array();
		for (const auto &action : actions)
			if (action.first) state["actions"].push_back(action.first.get()->encodeJSON());
	}

	if (!state.empty()) ticks_[std::to_string(tick)] = state;

	lastTickCount_ = tick;
	ReplayStream::instance().addTick(tick, state);
}

void ReplayEncoder::registerExpectedTeam(unsigned int teamId)
{
	if (!teamData_.count(teamId)) teamData_[teamId].teamId = teamId;
}
void ReplayEncoder::setTeamName(unsigned int teamId, const std::string &teamName)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].teamName = teamName;
}
std::string ReplayEncoder::getTeamNameFromTeamId(unsigned int teamId)
{
	if (!teamData_.count(teamId)) return "";
	return ReplayEncoder::instance().teamData_[teamId].teamName;
}
void ReplayEncoder::markConnectedInitially(unsigned int teamId, bool connected)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].connectedInitially = connected;
}
void ReplayEncoder::setDeathReason(unsigned int teamId, death_reason_t reason)
{
	registerExpectedTeam(teamId);
	if (static_cast<int>(teamData_[teamId].deathReason) <
		static_cast<int>(death_reason_t::TIMEOUT_SENDING_DATA)) // dont overwrite timeout death reasons
		teamData_[teamId].deathReason = reason;
}
void ReplayEncoder::setPlace(unsigned int teamId, unsigned int place)
{
	registerExpectedTeam(teamId);
	teamData_[teamId].place = place;
}

bool ReplayEncoder::wasConnectedInitially(unsigned int teamId) const
{
	auto it = teamData_.find(teamId);
	return it != teamData_.end() && it->second.connectedInitially;
}

void ReplayEncoder::includeConfig(json &config)
{
	config_ = config;
}

void ReplayEncoder::initializeReplayStream() const
{
	ReplayStream::instance().configure(config_, encodeMiscSection(false));
}

void ReplayEncoder::verifyReplaySaveFolder()
{
	std::vector<std::string> validReplaySaveFolders;
	for (const std::string &replaySaveFolder : Config::server().replayFolderPaths)
	{
		if (replaySaveFolder.empty())
		{
			Logger::Log(LogLevel::WARNING, "One replay save folder is not set.");
			continue;
		}

		if (!std::filesystem::exists(replaySaveFolder) || !std::filesystem::is_directory(replaySaveFolder))
		{
			Logger::Log(LogLevel::WARNING, "One replay save folder is incorrectly set to: " + replaySaveFolder);
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

json ReplayEncoder::encodeMiscSection(bool finished) const
{
	json miscSection;

	json players = json::array();
	for (const auto &kv : teamData_)
	{
		const auto &p = kv.second;
		json pj;
		pj["id"] = p.teamId;
		pj["name"] = p.teamName;
		if (finished)
		{
			pj["place"] = p.place;
			if (p.connectedInitially)
				pj["death_reason"] = static_cast<int>(p.deathReason);
			else
				pj["death_reason"] = static_cast<int>(death_reason_t::DID_NOT_CONNECT);
		}
		players.push_back(pj);
	}
	miscSection["team_results"] = players;

	miscSection["version"] = REPLAY_VERSION;
	miscSection["game_id"] = gameId_;
	if (finished) miscSection["stats"] = Stats::instance().toJson();
	for (auto &kv : customData_.items())
		miscSection[kv.key()] = kv.value();

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
	replayData["game_id"] = gameId_;
	replayData["misc"] = encodeMiscSection(true);
	replayData["ticks"] = !ticks_.empty() ? ticks_ : json::array();
	replayData["config"] = config_;
	replayData["full_tick_amount"] = lastTickCount_;

	const json misc = replayData["misc"];
	saveReplay(replayData);
	ReplayStream::instance().finish(misc, lastTickCount_);
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
