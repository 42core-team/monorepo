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

// Global variables to be used by the signal handler
volatile sig_atomic_t stop_server = 0;  // A flag to denote interruption
int server_fd_global = -1;  // The server socket file descriptor

void handle_sigint(int signum)
{
	(void)signum;

	write(STDOUT_FILENO, "\n[Main] SIGINT received. Initiating graceful shutdown...\n", 57);
	stop_server = 1;
	if (server_fd_global != -1)
	{
		close(server_fd_global);  // Close the server socket to free the port immediately
		server_fd_global = -1;
	}
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		std::cerr << "Usage: " << argv[0] << " <teamId1> <teamId2>\n";
		return 1;
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_sigint;
	sa.sa_flags = 0;
	if (sigaction(SIGINT, &sa, nullptr) == -1)
	{
		std::cerr << "[Main] Error: sigaction failed. " << strerror(errno) << std::endl;
		return 1;
	}

	std::vector<unsigned int> expectedTeamIds;
	for (int i = 1; i < argc; i++)
		expectedTeamIds.push_back(std::stoi(argv[i]));
	std::cout << "[Main] Expected team IDs: ";
	for (unsigned int teamId : expectedTeamIds)
		std::cout << teamId << " ";
	std::cout << std::endl;

	server_fd_global = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd_global < 0)
	{
		std::cerr << "[Main] Error: Could not create socket.\n";
		return 1;
	}
	int opt = 1;
	if (setsockopt(server_fd_global, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "[Main] Error: setsockopt failed.\n";
		close(server_fd_global);
		return 1;
	}

	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(4243);

	if (bind(server_fd_global, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		std::cerr << "[Main] Error: bind failed.\n";
		close(server_fd_global);
		return 1;
	}

	if (listen(server_fd_global, SOMAXCONN) < 0)
	{
		std::cerr << "[Main] Error: listen failed.\n";
		close(server_fd_global);
		return 1;
	}

	// GameConfig config = defaultConfig();
	// Game game(config);

	// std::thread gameThread([&game]() {
	// 	game.run();
	// });

	std::cout << "┌────────────────────────────────────┐\n";
	std::cout << "│   Server listening on port 4242... │\n";
	std::cout << "└────────────────────────────────────┘\n";

	std::unordered_map<unsigned int, Bridge*> bridges;

	while (bridges.size() < expectedTeamIds.size() && !stop_server)
	{
		sockaddr_in clientAddress;
		socklen_t clientLen = sizeof(clientAddress);
		int client_fd = accept(server_fd_global, reinterpret_cast<sockaddr*>(&clientAddress), &clientLen);
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

	close(server_fd_global);
	return 0;
}
