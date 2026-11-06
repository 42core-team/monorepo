#include "Bridge.h"
#include "Config.h"
#include "Game.h"
#include "Logger.h"
#include "Registration.h"
#include "Server.h"
#include "json.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

using json = nlohmann::ordered_json;

static std::string sanitizeTeamName(const std::string &teamName, const std::string &defaultName = "Unnamed")
{
	constexpr size_t MAX_LEN = 16;
	std::string sanitized;
	sanitized.reserve(std::min(teamName.size(), MAX_LEN));
	for (unsigned char uc : teamName)
	{
		if ((std::isalnum(uc) != 0) || uc == '_' || uc == '.' || uc == '-')
		{
			sanitized += static_cast<char>(uc);
			if (sanitized.size() >= MAX_LEN) break;
		}
	}
	if (sanitized.empty())
	{
		Logger::LogWarn("Team name '" + teamName + "' is invalid, using default name '" + defaultName + "'");
		return defaultName;
	}
	return sanitized;
}

// Try to find website-provided team name from environment variable
static std::string getWebsiteProvidedTeamName(unsigned int teamId)
{
	const std::string envKey = "PLAYER_" + std::to_string(teamId) + "_NAME";
	const char *rawEnv = std::getenv(envKey.c_str());
	if (rawEnv && *rawEnv)
	{
		return sanitizeTeamName(rawEnv);
	}
	return {};
}


int main(int argc, char *argv[])
{
	if (argc < 6)
	{
		Logger::Log(LogLevel::ERROR, std::string("Usage: ") + argv[0] +
											 " [server config file path] [game config file path] [data folder path] "
											 "<teamId1> <teamId2> ... <teamIdN>");
		return 1;
	}

	Config::setServerConfigFilePath(argv[1]);
	Config::setGameConfigFilePath(argv[2]);
	Config::setDataFolderPath(argv[3]);
	Config::server();
	Config::game();
	json encodedConfig = Config::encodeConfig();
	ReplayEncoder::instance().includeConfig(encodedConfig);

	ReplayEncoder::verifyReplaySaveFolder();

	if (argc - 4 > (int)Config::game().corePositions.size())
	{
		Logger::Log(LogLevel::ERROR, "Too many team IDs for Core Locations specified.");
		return 1;
	}

	std::vector<unsigned int> expectedTeamIds;
	for (int i = 4; i < argc; i++)
		expectedTeamIds.push_back(std::stoi(argv[i]));

	std::sort(expectedTeamIds.begin(), expectedTeamIds.end());
	if (std::adjacent_find(expectedTeamIds.begin(), expectedTeamIds.end()) != expectedTeamIds.end())
	{
		Logger::Log(LogLevel::ERROR, "Duplicate team IDs specified.");
		return 1;
	}

	// register expected teams & set names based on env if available, otherwise fallback
	// so if a team never connects they still show up correctly in the replay
	for (unsigned int teamId : expectedTeamIds)
	{
		ReplayEncoder::instance().registerExpectedTeam(teamId);

		std::string teamName = getWebsiteProvidedTeamName(teamId);
		if (teamName.empty()) teamName = "Team" + std::to_string(teamId);
		ReplayEncoder::instance().setTeamName(teamId, teamName);
	}

	std::string teamIdsStr = "Expected team IDs: ";
	for (unsigned int teamId : expectedTeamIds)
		teamIdsStr += std::to_string(teamId) + " ";
	Logger::Log(teamIdsStr);

	// will throw a runtime error if something fails
	Server server{};

	std::unordered_map<unsigned int, std::unique_ptr<Bridge>> bridges{};
	for (auto &[teamId, client] : registerClients(server, expectedTeamIds))
	{
		// allow client to override only if current name is the fallback
		const std::string fallback = "Team" + std::to_string(teamId);
		if (ReplayEncoder::instance().getTeamNameFromTeamId(teamId) == fallback)
		{
			ReplayEncoder::instance().setTeamName(teamId, sanitizeTeamName(client.name, fallback));
		}
		ReplayEncoder::instance().markConnectedInitially(teamId, true);
		client.bridge->detectPeerClosed();
		bridges[teamId] = std::move(client.bridge);
	}
	size_t placeCounter = expectedTeamIds.size() - 1;
	std::vector<unsigned int> connectedTeamIds;
	for (unsigned int teamId : expectedTeamIds)
	{
		if (bridges.find(teamId) == bridges.end())
		{
			ReplayEncoder::instance().setDeathReason(teamId, death_reason_t::DID_NOT_CONNECT);
			ReplayEncoder::instance().setPlace(teamId, placeCounter--);
		}
	}
	for (unsigned int teamId : expectedTeamIds)
	{
		auto it = bridges.find(teamId);
		if (it != bridges.end() && it->second->isDisconnected())
		{
			ReplayEncoder::instance().setDeathReason(teamId, death_reason_t::DISCONNECTED);
			ReplayEncoder::instance().setPlace(teamId, placeCounter--);
			bridges.erase(it);
		}
		else if (it != bridges.end())
		{
			connectedTeamIds.push_back(teamId);
		}
	}

	Logger::Log("Preparing to start the game with " + std::to_string(connectedTeamIds.size()) + " / " +
				std::to_string(expectedTeamIds.size()) + " teams connected.");

	Game game(connectedTeamIds);

	for (auto &pair : bridges)
	{
		pair.second->start();
		game.addBridge(std::move(pair.second));
	}

	game.run();

	return 0;
}
