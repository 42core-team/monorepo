#include "Bridge.h"

#include <sys/socket.h>  // For shutdown()
#include <unistd.h>      // For close(), read() and write()
#include <cstring>       // For strerror()
#include <iostream>
#include <stdexcept>
#include <errno.h>

Bridge::Bridge(int socket_fd)
    : socket_fd_(socket_fd), disconnected_(false)
{
    // Set a timeout of 1 second on the socket read operations.
    struct timeval tv;
    tv.tv_sec = 1;  // one second
    tv.tv_usec = 0;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        std::cerr << "[Bridge] Error setting socket timeout: " << strerror(errno) << "\n";
    }
}

Bridge::~Bridge() {
    // Mark the connection as severed.
    disconnected_ = true;

	// wake up threads waiting on condition variables
	writeCv_.notify_all();
    readCv_.notify_all();
    // A graceful shutdown of the socket for both reading and writing.
    shutdown(socket_fd_, SHUT_RDWR);
    // Ensure threads are properly joined.
    if (readThread_.joinable()) readThread_.join();
    if (writeThread_.joinable()) writeThread_.join();
    close(socket_fd_);
}

void Bridge::start() {
    // Spawn the read and write threads.
    readThread_ = std::thread(&Bridge::readLoop, this);
    writeThread_ = std::thread(&Bridge::writeLoop, this);
}

void Bridge::sendMessage(const json& message) {
    // Append a newline as a delimiter to the JSON string.
    std::string msg = message.dump() + "\n";
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        writeQueue_.push(msg);
    }
    writeCv_.notify_one();
}

bool Bridge::receiveMessage(json& message) {
    std::unique_lock<std::mutex> lock(readMutex_);
    // Block until a message is available or the connection is severed.
    if (readQueue_.empty()) {
        readCv_.wait(lock, [this]{ return !readQueue_.empty() || disconnected_; });
    }
    if (readQueue_.empty()) return false;
    message = readQueue_.front();
    readQueue_.pop();
	std::cout << "Received message: " << message << std::endl;
    return true;
}

bool Bridge::isDisconnected() {
    return disconnected_;
}

void Bridge::readLoop() {
    try {
        constexpr size_t buffer_size = 1024;
        char buffer[buffer_size];
        std::string data;  // This string accumulates partial data.
        while (!disconnected_) {
            ssize_t n = ::read(socket_fd_, buffer, buffer_size);
            if (n <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Timeout reached; no data is available. Continue without flagging an error.
                    continue;
                } else {
                    std::cerr << "[Bridge::readLoop] Read error: " << strerror(errno) << "\n";
                    disconnected_ = true;
                    break;
                }
            }
            data.append(buffer, n);
            // Process complete lines separated by newline characters.
            size_t pos;
            while ((pos = data.find('\n')) != std::string::npos) {
                std::string line = data.substr(0, pos);
                data.erase(0, pos + 1);
                if (!line.empty()) {
                    try {
                        json j = json::parse(line);
                        {
                            std::lock_guard<std::mutex> lock(readMutex_);
                            readQueue_.push(j);
                        }
                        readCv_.notify_one();
                    } catch (json::parse_error& e) {
                        std::cerr << "[Bridge::readLoop] JSON parse error: " << e.what() << "\n";
                    }
                }
            }
        }
    } catch (std::exception& e) {
        std::cerr << "[Bridge::readLoop] Exception: " << e.what() << "\n";
        disconnected_ = true;
    }
}

void Bridge::writeLoop() {
    try {
        while (!disconnected_) {
            std::unique_lock<std::mutex> lock(writeMutex_);
            // Wait until there is a message to send or disconnection occurs.
            writeCv_.wait(lock, [this] { return !writeQueue_.empty() || disconnected_; });
            while (!writeQueue_.empty()) {
                std::string msg = writeQueue_.front();
                writeQueue_.pop();
                // Unlock while performing the potentially blocking I/O operation.
                lock.unlock();
                const char* data = msg.c_str();
                size_t remaining = msg.size();
                while (remaining > 0) {
                    ssize_t n = ::write(socket_fd_, data, remaining);
                    if (n < 0) {
                        std::cerr << "[Bridge::writeLoop] Write error: " << strerror(errno) << "\n";
                        disconnected_ = true;
                        break;
                    }
                    remaining -= n;
                    data += n;
                }
                lock.lock();
            }
        }
    } catch (std::exception& e) {
        std::cerr << "[Bridge::writeLoop] Exception: " << e.what() << "\n";
        disconnected_ = true;
    }
}
