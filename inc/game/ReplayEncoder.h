#ifndef REPLAY_ENCODER_H
#define REPLAY_ENCODER_H

#include "json.hpp"
using json = nlohmann::ordered_json;

#include "Logger.h"

#include <fstream>

class ReplayEncoder
{
public:
	ReplayEncoder() : ticks_(json::object()), lastTickCount_(0) {}
	~ReplayEncoder() = default;

	void addTickState(const json& state);
	void includeConfig(json& config);
	void saveReplay() const;

	static void setReplaySaveFolder(const std::string& folder);
	static void verifyReplaySaveFolder();

private:
	json diffObjects(const json& currentObjects);
	json diffObject(const json& currentObj, const json& previousObj);

	json ticks_;
	json config_;

	std::unordered_map<int, json> previousObjects_;

	unsigned long long lastTickCount_;

	static std::string replaySaveFolder_;
};

#endif // REPLAY_ENCODER_H
