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

#include "json.hpp"
using json = nlohmann::ordered_json;

int main(int argc, char *argv[])
{
	if (argc < 5)
	{
		Logger::Log(LogLevel::ERROR, std::string("Usage: ") + argv[0] + " [config file path] [replay save folder] <teamId1> <teamId2> ...");
		return 1;
	}

	Config::setConfigFilePath(argv[1]);
	Config::instance(); // Would exit if config file is invalid & initializes config

	ReplayEncoder::setReplaySaveFolder(argv[2]);
	ReplayEncoder::verifyReplaySaveFolder();

	if (argc - 3 > (int)Config::instance().corePositions.size())
	{
		Logger::Log(LogLevel::ERROR, "Too many team IDs for Core Locations specified.");
		return 1;
	}

	srand(time(nullptr));

	std::vector<unsigned int> expectedTeamIds;
	for (int i = 3; i < argc; i++)
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

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not create socket.");
		return 1;
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not set socket options.");
		close(server_fd);
		return 1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(4242);

	if (bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not bind socket.");
		close(server_fd);
		return 1;
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not listen on socket.");
		close(server_fd);
		return 1;
	}

	Logger::Log("Server listening on port 4242...");

	std::unordered_map<unsigned int, std::unique_ptr<Bridge>> bridges{};

	while (bridges.size() < expectedTeamIds.size())
	{
		sockaddr_in clientAddress;
		socklen_t clientLen = sizeof(clientAddress);
		int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&clientAddress), &clientLen);
		if (client_fd < 0)
		{
			Logger::Log(LogLevel::WARNING, "accept failed: " + std::string(strerror(errno)));
			continue;
		}

		Logger::Log("Accepted connection from " + std::string(inet_ntoa(clientAddress.sin_addr)));

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

	close(server_fd);
	return 0;
}
