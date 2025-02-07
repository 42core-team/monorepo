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

#include "json.hpp"
using json = nlohmann::json;

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <teamId1> <teamId2>\n";
		return 1;
	}

	if (argc - 1 > Config::getInstance().corePositions.size())
	{
		std::cerr << "[Main] Error: too many team IDs specified.\n";
		return 1;
	}

	std::vector<unsigned int> expectedTeamIds;
	for (int i = 1; i < argc; i++)
		expectedTeamIds.push_back(std::stoi(argv[i]));
	std::cout << "[Main] Expected team IDs: ";
	for (unsigned int teamId : expectedTeamIds)
		std::cout << teamId << " ";
	std::cout << std::endl;

	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0)
	{
		std::cerr << "[Main] Error: Could not create socket.\n";
		return 1;
	}
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "[Main] Error: setsockopt failed.\n";
		close(server_fd);
		return 1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(4243);

	if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		std::cerr << "[Main] Error: bind failed.\n";
		close(server_fd);
		return 1;
	}

	if (listen(server_fd, SOMAXCONN) < 0)
	{
		std::cerr << "[Main] Error: listen failed.\n";
		close(server_fd);
		return 1;
	}

	std::cout << "┌────────────────────────────────────┐\n";
	std::cout << "│   Server listening on port 4242... │\n";
	std::cout << "└────────────────────────────────────┘\n";

	std::unordered_map<unsigned int, Bridge*> bridges;

	while (bridges.size() < expectedTeamIds.size())
	{
		sockaddr_in clientAddress;
		socklen_t clientLen = sizeof(clientAddress);
		int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
		if (client_fd < 0)
		{
			std::cerr << "[Main] Error: accept failed." << std::endl;
			continue;
		}

		std::cout << "[Main] Accepted connection from " << inet_ntoa(clientAddress.sin_addr) << std::endl;
		
		Bridge* bridge = new Bridge(client_fd);
		bridge->start();

		json loginMessage;
		if (!bridge->receiveMessage(loginMessage))
		{
			std::cerr << "[Main] Error: did not receive a login message from the client." << std::endl;
			delete bridge;
			continue;
		}
		if (!loginMessage.contains("password") || loginMessage["password"] != "42") // very important and secure authentication
		{
			std::cerr << "[Main] Error: incorrect password." << std::endl;
			delete bridge;
			continue;
		}
		unsigned int teamId = loginMessage["id"];

		if (std::find(expectedTeamIds.begin(), expectedTeamIds.end(), teamId) == expectedTeamIds.end())
		{
			std::cerr << "[Main] Error: received unexpected team id " << teamId << std::endl;
			delete bridge;
			continue;
		}
		if (bridges.find(teamId) != bridges.end())
		{
			std::cerr << "[Main] Error: duplicate connection for team id " << teamId << std::endl;
			delete bridge;
			continue;
		}

		bridges[teamId] = bridge;
		std::cout << "[Main] Accepted team " << teamId << std::endl;
	}

	std::cout << "[Main] All expected teams have connected. Preparing to start the game..." << std::endl;

	GameConfig config = defaultConfig();
	Game game(config);
	
	for (auto& pair : bridges)
	{
		game.addBridge(pair.second);
	}

	std::thread gameThread([&game]()
	{
		game.run();
	});

	gameThread.join();

	close(server_fd);
	return 0;
}
