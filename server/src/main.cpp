#include "Config.h"
#include "Game.h"
#include "GameServiceImpl.h"
#include "GrpcServer.h"
#include "Logger.h"
#include "json.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

using json = nlohmann::ordered_json;

// Try to find website-provided team name from environment variable
static std::string getWebsiteProvidedTeamName(unsigned int teamId)
{
	const std::string envKey = "PLAYER_" + std::to_string(teamId) + "_NAME";
	const char *rawEnv = std::getenv(envKey.c_str());
	if (rawEnv && *rawEnv)
	{
		// sanitize inline (same logic as in GameServiceImpl)
		constexpr size_t MAX_LEN = 16;
		std::string sanitized;
		std::string raw(rawEnv);
		sanitized.reserve(std::min(raw.size(), MAX_LEN));
		for (unsigned char uc : raw)
		{
			if ((std::isalnum(uc) != 0) || uc == '_' || uc == '.' || uc == '-')
			{
				sanitized += static_cast<char>(uc);
				if (sanitized.size() >= MAX_LEN) break;
			}
		}
		if (!sanitized.empty()) return sanitized;
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

	// Register expected teams & set names based on env if available
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

	// Create gRPC service and server
	GameServiceImpl service(expectedTeamIds);
	GrpcServer grpcServer(&service);

	// Start gRPC server in a background thread
	std::thread grpcThread([&grpcServer]() { grpcServer.start(); });

	// Wait for all clients to login (or timeout)
	bool allConnected = service.waitForAllLogins(Config::server().clientConnectTimeoutMs);
	if (!allConnected)
	{
		Logger::LogWarn("Not all teams connected within timeout.");
	}

	// Handle non-connected teams
	size_t placeCounter = expectedTeamIds.size() - 1;
	std::vector<unsigned int> connectedTeamIds = service.getConnectedTeamIds();
	for (unsigned int teamId : expectedTeamIds)
	{
		bool isConnected = std::find(connectedTeamIds.begin(), connectedTeamIds.end(), teamId) != connectedTeamIds.end();
		if (!isConnected)
		{
			ReplayEncoder::instance().setDeathReason(teamId, death_reason_t::DID_NOT_CONNECT);
			ReplayEncoder::instance().setPlace(teamId, placeCounter--);
		}
	}

	// Check for disconnected sessions among connected ones
	for (unsigned int teamId : connectedTeamIds)
	{
		auto session = service.getSession(teamId);
		if (session && session->isDisconnected())
		{
			ReplayEncoder::instance().setDeathReason(teamId, death_reason_t::DISCONNECTED);
			ReplayEncoder::instance().setPlace(teamId, placeCounter--);
			// Remove from active list
			connectedTeamIds.erase(
					std::remove(connectedTeamIds.begin(), connectedTeamIds.end(), teamId), connectedTeamIds.end());
		}
	}

	Logger::Log("Preparing to start the game with " + std::to_string(connectedTeamIds.size()) + " / " +
				std::to_string(expectedTeamIds.size()) + " teams connected.");

	Game game(connectedTeamIds, &service);
	game.run();

	grpcServer.shutdown();
	grpcThread.join();

	return 0;
}
