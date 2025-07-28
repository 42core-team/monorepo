#include <memory>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <csignal>
#include <cerrno>

#include "Bridge.h"
#include "Game.h"
#include "Config.h"
#include "Logger.h"
#include "Server.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		Logger::Log(LogLevel::ERROR, std::string("Usage: ") + argv[0] + " [server config file path] <teamId1> <teamId2> ...");
		return 1;
	}

	Config::setServerConfigFilePath(argv[1]);
	Config::game(); // Would exit if config file is invalid & initializes config
	Config::server();

	ReplayEncoder::verifyReplaySaveFolder();

	if (argc - 2 > (int)Config::game().corePositions.size())
	{
		Logger::Log(LogLevel::ERROR, "Too many team IDs for Core Locations specified.");
		return 1;
	}

	// init srand
	srand(time(nullptr));

	std::vector<unsigned int> expectedTeamIds;
	for (int i = 2; i < argc; i++)
		expectedTeamIds.push_back(std::stoi(argv[i]));

	std::sort(expectedTeamIds.begin(), expectedTeamIds.end());
	if (std::adjacent_find(expectedTeamIds.begin(), expectedTeamIds.end()) != expectedTeamIds.end())
	{
		Logger::Log(LogLevel::ERROR, "Duplicate team IDs specified.");
		return 1;
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
			continue;
		}

		Logger::Log("Accepted connection from " + client_addr);

		std::unique_ptr<Bridge> bridge = std::make_unique<Bridge>(client_fd, expectedTeamIds[bridges.size()]);
		bridge->start();

		json loginMessage;
		if (!bridge->receiveMessage(loginMessage))
		{
			Logger::Log(LogLevel::WARNING, "Did not receive a login message from the client.");
			continue;
		}
		if (!loginMessage.contains("password") || loginMessage["password"] != "42") // very important and secure authentication
		{
			Logger::Log(LogLevel::WARNING, "Incorrect password.");
			continue;
		}
		unsigned int teamId = loginMessage["id"];
		std::string teamName = loginMessage["name"];
		bridge->setTeamId(teamId);
		bridge->setTeamName(teamName);

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
	}

	Logger::Log("All expected teams have connected. Preparing to start the game...");

	Game game(expectedTeamIds);

	for (auto& pair : bridges)
		game.addBridge(std::move(pair.second));

	std::thread gameThread([&game]()
						   { game.run(); });

	gameThread.join();

	return 0;
}
