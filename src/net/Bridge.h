#ifndef BRIDGE_H
#define BRIDGE_H

#include <boost/asio.hpp>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>
#include "json.hpp"  // nlohmann/json

using json = nlohmann::json;
using boost::asio::ip::tcp;

class Bridge {
public:
    Bridge(boost::asio::io_context& io_context, tcp::socket socket);
    ~Bridge();

    // Send a JSON message over the socket.
    void sendMessage(const json& message);
    // Blocking call: receive a JSON message (returns false if disconnected).
    bool receiveMessage(json& message);
    // Check if the connection is disconnected.
    bool isDisconnected();

    // Start the asynchronous read and write threads.
    void start();

private:
    void readLoop();
    void writeLoop();

    tcp::socket socket_;
    std::thread readThread_;
    std::thread writeThread_;

    std::queue<std::string> writeQueue_;
    std::mutex writeMutex_;
    std::condition_variable writeCv_;

    std::queue<json> readQueue_;
    std::mutex readMutex_;
    std::condition_variable readCv_;

    std::atomic<bool> disconnected_;
};

#endif // BRIDGE_H
