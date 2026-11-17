#ifndef TEAM_PORTS_H
#define TEAM_PORTS_H

#include <map>
#include <string>
#include <vector>

/**
 * Parses a TEAM_PORTS value of comma-separated <teamId>=<port> pairs, e.g. "123456=4501,654321=4502".
 * Every expected team needs exactly one entry and every port must be distinct. No whitespace is allowed.
 *
 * @return The port of each team, empty if value is empty (one shared listener for all teams).
 * @throws std::invalid_argument describing the first problem found.
 */
std::map<unsigned int, int> parseTeamPorts(const std::string &value, const std::vector<unsigned int> &expectedTeamIds);

/**
 * Reads TEAM_PORTS from the environment, see parseTeamPorts(). Unset behaves like empty.
 */
std::map<unsigned int, int> readTeamPorts(const std::vector<unsigned int> &expectedTeamIds);

#endif // TEAM_PORTS_H
