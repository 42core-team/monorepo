#ifndef REPLAY_ENCODER_H
#define REPLAY_ENCODER_H

#include "json.hpp"
using json = nlohmann::ordered_json;

class ReplayEncoder
{
public:
	ReplayEncoder() : ticks_(json::object()), lastTickCount_(0) {}
	~ReplayEncoder() = default;

	void addTickState(const json& state);
	json getReplay() const;

private:
	json diffObjects(const json& currentObjects);
	json diffObject(const json& currentObj, const json& previousObj);

	json ticks_;

	std::unordered_map<int, json> previousObjects_;

	unsigned long long lastTickCount_;
};

#endif // REPLAY_ENCODER_H
