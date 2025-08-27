#ifndef STATS_H
#define STATS_H

#include <unordered_map>
#include <string>
#include <string_view>
#include "json.hpp"
using json = nlohmann::ordered_json;

namespace stat_keys
{
	inline constexpr std::string_view damage_total = "damage_total";
	inline constexpr std::string_view damage_self = "damage_self";
	inline constexpr std::string_view damage_opponent = "damage_opponent";

	inline constexpr std::string_view damage_units = "damage_units";
	inline constexpr std::string_view damage_cores = "damage_cores";
	inline constexpr std::string_view damage_walls = "damage_walls";
	inline constexpr std::string_view damage_deposits = "damage_deposits";

	inline constexpr std::string_view units_spawned = "units_spawned";
	inline constexpr std::string_view walls_spawned = "walls_spawned";
	inline constexpr std::string_view bombs_spawned = "bombs_spawned";

	inline constexpr std::string_view units_destroyed = "units_destroyed";
	inline constexpr std::string_view cores_destroyed = "cores_destroyed";
	inline constexpr std::string_view deposits_destroyed = "deposits_destroyed";
	inline constexpr std::string_view gempiles_destroyed = "gempiles_destroyed"; // picked up
	inline constexpr std::string_view walls_destroyed = "walls_destroyed";
	inline constexpr std::string_view bombs_destroyed = "bombs_destroyed"; // explosions

	inline constexpr std::string_view money_transferred = "money_transferred";
	inline constexpr std::string_view tiles_traveled = "tiles_traveled";

	inline constexpr std::string_view gems_gained = "gems_gained";

	inline constexpr std::string_view actions_executed = "actions_executed";
}

class Stats
{
public:
	static Stats &instance();
	void inc(std::string_view key, uint64_t by = 1);
	json toJson() const;

private:
	std::unordered_map<std::string, uint64_t> counters_;
};

#endif // STATS_H
