#ifndef BRIDGE_H
#define BRIDGE_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>

#include "json.hpp"  // nlohmann/json

using json = nlohmann::json;

class Bridge {
public:
    explicit Bridge(int socket_fd);
    ~Bridge();

    // Transmit a JSON message over the socket.
    void sendMessage(const json& message);
    // Blocking call to receive a JSON message (returns false if disconnected).
    bool receiveMessage(json& message);
    // Returns the disconnection status.
    bool isDisconnected();

    // Begin the asynchronous read and write threads.
    void start();

private:
    void readLoop();
    void writeLoop();

    int socket_fd_;
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
