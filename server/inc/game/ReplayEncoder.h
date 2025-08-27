#ifndef REPLAY_ENCODER_H
#define REPLAY_ENCODER_H

#include "json.hpp"
using json = nlohmann::ordered_json;

#include "Logger.h"
#include "Config.h"
#include "Action.h"
#include "Stats.h"
#include "Core.h"

#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <cstdlib>

enum class death_reason_t
{
	NONE_SURVIVED,
	CORE_DESTROYED,
	DISCONNECTED,
	DID_NOT_CONNECT,
	TIMEOUT_SENDING_DATA,
	TIMEOUT_CORE_HP,
	TIMEOUT_UNIT_HP,
	TIMEOUT_RANDOM
};

typedef struct team_data_s
{
	unsigned int teamId;
	std::string teamName;
	bool connectedInitially = false;
	death_reason_t deathReason = death_reason_t::DID_NOT_CONNECT;
	unsigned int place = std::numeric_limits<unsigned int>::max(); // 0 = won
} team_data_t;

class ReplayEncoder
{
public:
	ReplayEncoder() : ticks_(json::object()), lastTickCount_(0) {}
	~ReplayEncoder() = default;

	static ReplayEncoder &instance();

	void addTickState(json &state, unsigned long long tick, std::vector<std::pair<std::unique_ptr<Action>, Core *>> &actions);

	void registerExpectedTeam(unsigned int teamId);
	void setTeamName(unsigned int teamId, const std::string &teamName);
	void markConnectedInitially(unsigned int teamId, bool connected);
	void setDeathReason(unsigned int teamId, death_reason_t reason);
	void setPlace(unsigned int teamId, unsigned int place);

	bool wasConnectedInitially(unsigned int teamId) const;

	void includeConfig(json &config);
	json &getCustomData(void) { return customData_; }

	void exportReplay() const;
	void saveReplay(const json &replayData) const;

	static void verifyReplaySaveFolder();

private:
	json encodeMiscSection() const;

	json ticks_ = json::array();
	json config_ = json::object();
	json customData_ = json::object();

	unsigned long long lastTickCount_;
	std::unordered_map<unsigned int, team_data_t> teamData_;
};

#endif // REPLAY_ENCODER_H
