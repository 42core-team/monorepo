#include "Bridge.h"
#include "Config.h"
#include "Game.h"
#include "Logger.h"
#include "Server.h"
#include "json.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <iostream>
#include <json-schema.hpp>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>
using nlohmann::json_schema::json_validator;

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

	while (bridges.size() < expectedTeamIds.size())
	{
		std::string client_addr;
		int client_fd = server.acceptClient(client_addr);
		if (client_fd < 0)
		{
			Logger::Log(LogLevel::WARNING, "accept failed: " + std::string(strerror(errno)));
			break;
		}

		Logger::Log("Accepted connection from " + client_addr);

		std::unique_ptr<Bridge> bridge = std::make_unique<Bridge>(client_fd, expectedTeamIds[bridges.size()]);
		bridge->start();

		// handle login message
		json loginMessage;
		if (!bridge->receiveMessage(loginMessage))
		{
			Logger::Log(LogLevel::WARNING, "Did not receive a login message from the client.");
			continue;
		}
		try
		{
			json_validator v;
			v.set_root_schema(Config::load_json_schema("packets/login-message.schema.json"));
			v.validate(loginMessage);
		}
		catch (const std::exception &e)
		{
			Logger::Log(LogLevel::WARNING, std::string("Login message validation error: ") + e.what() + " (\"" +
												   loginMessage.dump() + "\")");
			continue;
		}
		unsigned int teamId = loginMessage["id"];
		bridge->setTeamId(teamId);

		// set team name
		std::string clientName =
				sanitizeTeamName(static_cast<std::string>(loginMessage["name"]), "Team" + std::to_string(teamId));
		// allow client to override only if current name is the fallback
		const std::string current = ReplayEncoder::instance().getTeamNameFromTeamId(teamId);
		const std::string fallback = "Team" + std::to_string(teamId);
		if (current == fallback)
		{
			ReplayEncoder::instance().setTeamName(teamId, clientName);
		}

		if (std::find(expectedTeamIds.begin(), expectedTeamIds.end(), teamId) == expectedTeamIds.end())
		{
			Logger::Log(LogLevel::WARNING, "Received unexpected team ID " + std::to_string(teamId));
			continue;
		}
		if (bridges.find(teamId) != bridges.end())
		{
			Logger::Log(LogLevel::WARNING, "Duplicate connection for team ID " + std::to_string(teamId));
			continue;
		}

		bridges[teamId] = std::move(bridge);
		Logger::Log("Accepted team " + std::to_string(teamId));
		ReplayEncoder::instance().markConnectedInitially(teamId, true);
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
		}
		else if (it != bridges.end())
		{
			connectedTeamIds.push_back(teamId);
		}
	}

	Logger::Log("Preparing to start the game with " + std::to_string(connectedTeamIds.size()) + " / " +
				std::to_string(expectedTeamIds.size()) + " teams connected.");

	Game game(connectedTeamIds);
	ReplayEncoder::instance().startLiveUpdates(Config::server().liveReplayPort);

	for (auto &pair : bridges)
		game.addBridge(std::move(pair.second));

	game.run();

	return 0;
}
