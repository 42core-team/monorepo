#include "Stats.h"

Stats &Stats::instance()
{
	static Stats s;
	return s;
}

void Stats::inc(std::string_view key, uint64_t by)
{
	counters_[std::string(key)] += by;
}

json Stats::toJson() const
{
	json j = json::object();
	for (const auto &kv : counters_)
		j[kv.first] = kv.second;
	return j;
}
