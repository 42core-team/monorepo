#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <cstring>
#include "Bridge.h"
#include "game/Game.h"
#include "game/Config.h"

int main() {
    // Create a TCP socket.
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "[Main] Error: Could not create socket.\n";
        return 1;
    }

    // Allow immediate reuse of the port after program termination.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "[Main] Error: setsockopt failed.\n";
        close(server_fd);
        return 1;
    }

    // Define the server address.
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all available interfaces.
    address.sin_port = htons(4242);

    // Bind the socket to the specified port.
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "[Main] Error: bind failed.\n";
        close(server_fd);
        return 1;
    }

    // Start listening for incoming connections.
    if (listen(server_fd, SOMAXCONN) < 0) {
        std::cerr << "[Main] Error: listen failed.\n";
        close(server_fd);
        return 1;
    }

    // Initialize your game.
    GameConfig config = defaultConfig();
    Game game(config);

    // Launch the game loop in a dedicated thread.
    std::thread gameThread([&game]() {
        game.run();
    });

    std::cout << "┌────────────────────────────────────┐\n";
    std::cout << "│ Server listening on port 4242...   │\n";
    std::cout << "└────────────────────────────────────┘\n";

    // Accept connections indefinitely.
    while (true) {
        struct sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int client_fd = accept(server_fd, (struct sockaddr*)&clientAddress, &clientLen);
        if (client_fd < 0) {
            std::cerr << "[Main] Error: accept failed.\n";
            continue;
        }
        std::cout << "[Main] Accepted connection from "
                  << inet_ntoa(clientAddress.sin_addr) << "\n";

        // Instantiate a new Bridge with the accepted socket.
        Bridge* bridge = new Bridge(client_fd);
        bridge->start();
        // Incorporate the new Bridge into your game.
        game.addBridge(bridge);
    }

    // Clean up.
    gameThread.join();
    close(server_fd);
    return 0;
}
