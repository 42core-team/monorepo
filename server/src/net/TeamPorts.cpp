#include "TeamPorts.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <set>
#include <sstream>
#include <stdexcept>

namespace
{
constexpr unsigned long MAX_PORT = 65535;

// Accepts plain decimal digits only, so signs, spaces and hex never slip through.
unsigned long parseNumber(const std::string &text, unsigned long max, const std::string &what, const std::string &entry)
{
	if (text.empty() || text.size() > 10 ||
		!std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c) != 0; }))
		throw std::invalid_argument("invalid " + what + " in entry '" + entry + "'");
	const unsigned long long number = std::stoull(text);
	if (number > max) throw std::invalid_argument(what + " out of range in entry '" + entry + "'");
	return static_cast<unsigned long>(number);
}
} // namespace

std::map<unsigned int, int> parseTeamPorts(const std::string &value, const std::vector<unsigned int> &expectedTeamIds)
{
	std::map<unsigned int, int> teamPorts;
	if (value.empty()) return teamPorts;

	std::set<int> ports;
	std::stringstream entries(value + ","); // getline drops a trailing empty entry, the extra comma keeps it
	std::string entry;
	while (std::getline(entries, entry, ','))
	{
		const size_t separator = entry.find('=');
		if (separator == std::string::npos) throw std::invalid_argument("entry '" + entry + "' is not <teamId>=<port>");

		const auto teamId = static_cast<unsigned int>(
				parseNumber(entry.substr(0, separator), std::numeric_limits<unsigned int>::max(), "team ID", entry));
		const auto port = static_cast<int>(parseNumber(entry.substr(separator + 1), MAX_PORT, "port", entry));
		if (port == 0) throw std::invalid_argument("port out of range in entry '" + entry + "'");

		if (std::find(expectedTeamIds.begin(), expectedTeamIds.end(), teamId) == expectedTeamIds.end())
			throw std::invalid_argument("team ID " + std::to_string(teamId) + " is not an expected team");
		if (!teamPorts.emplace(teamId, port).second)
			throw std::invalid_argument("team ID " + std::to_string(teamId) + " is listed more than once");
		if (!ports.insert(port).second)
			throw std::invalid_argument("port " + std::to_string(port) + " is assigned to more than one team");
	}

	for (unsigned int teamId : expectedTeamIds)
		if (teamPorts.find(teamId) == teamPorts.end())
			throw std::invalid_argument("expected team ID " + std::to_string(teamId) + " has no port");
	return teamPorts;
}

std::map<unsigned int, int> readTeamPorts(const std::vector<unsigned int> &expectedTeamIds)
{
	const char *rawEnv = std::getenv("TEAM_PORTS");
	return parseTeamPorts(rawEnv ? rawEnv : "", expectedTeamIds);
}
