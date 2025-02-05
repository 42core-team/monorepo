#include <boost/asio.hpp>
#include <iostream>
#include "game/Game.h"
#include "game/Config.h"
#include "net/Bridge.h"

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 4242));
        
        GameConfig config = defaultConfig();
        Game game(config);
        
        // Run the game loop in its own thread.
        std::thread gameThread([&game]() {
            game.run();
        });
        
        // Accept connections indefinitely.
        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);
            // Create a Bridge for the new connection.
            Bridge* bridge = new Bridge(io_context, std::move(socket));
            bridge->start();
            // Add the bridge to the game (teams and spectators will be handled similarly).
            game.addBridge(bridge);
        }
        
        gameThread.join();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    
    return 0;
}
