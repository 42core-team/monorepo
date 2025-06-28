#ifndef REPLAY_ENCODER_H
#define REPLAY_ENCODER_H

#include "json.hpp"
using json = nlohmann::ordered_json;

#include "Logger.h"

#include <fstream>

typedef struct team_data_s {
	unsigned int teamId;
	std::string teamName;
	unsigned int place; // 0 = won
}	team_data_t;

typedef enum game_end_reason_e {
	GER_DEFEAT,
	GER_CORE_HP,
	GER_UNIT_HP,
	GER_MONEY_IN_CORE,
	GER_RANDOM
}	game_end_reason_t;

class ReplayEncoder
{
public:
	ReplayEncoder() : ticks_(json::object()), lastTickCount_(0) {}
	~ReplayEncoder() = default;

	void addTickState(const json& state);
	void addTeamScore(unsigned int teamId, const std::string& teamName, unsigned int place);
	void setGameEndReason(game_end_reason_t reason) { gameEndReason_ = reason; }
	void includeConfig(json& config);
	void saveReplay() const;

	static void setReplaySaveFolder(const std::string& folder);
	static void verifyReplaySaveFolder();

private:
	json diffObjects(const json& currentObjects);
	json diffObject(const json& currentObj, const json& previousObj);

	json encodeMiscSection() const;

	json ticks_;
	json config_;

	std::unordered_map<int, json> previousObjects_;
	unsigned long long lastTickCount_;
	std::vector<team_data_t> teamData_;
	game_end_reason_t gameEndReason_ = GER_DEFEAT;

	static std::string replaySaveFolder_;
};

#endif // REPLAY_ENCODER_H
