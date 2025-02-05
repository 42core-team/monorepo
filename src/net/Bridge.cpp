#include "Bridge.h"
#include <iostream>

Bridge::Bridge(boost::asio::io_context& io_context, tcp::socket socket)
    : socket_(std::move(socket)), disconnected_(false)
{
}

Bridge::~Bridge() {
    disconnected_ = true;
    if (readThread_.joinable()) readThread_.join();
    if (writeThread_.joinable()) writeThread_.join();
}

void Bridge::start() {
    readThread_ = std::thread(&Bridge::readLoop, this);
    writeThread_ = std::thread(&Bridge::writeLoop, this);
}

void Bridge::sendMessage(const json& message) {
    std::string msg = message.dump() + "\n";
    {
        std::lock_guard<std::mutex> lock(writeMutex_);
        writeQueue_.push(msg);
    }
    writeCv_.notify_one();
}

bool Bridge::receiveMessage(json& message) {
    std::unique_lock<std::mutex> lock(readMutex_);
    if (readQueue_.empty()) {
        readCv_.wait(lock, [this]{ return !readQueue_.empty() || disconnected_; });
    }
    if (readQueue_.empty()) return false;
    message = readQueue_.front();
    readQueue_.pop();
    return true;
}

bool Bridge::isDisconnected() {
    return disconnected_;
}

void Bridge::readLoop() {
    try {
        boost::asio::streambuf buffer;
        while (!disconnected_) {
            boost::asio::read_until(socket_, buffer, "\n");
            std::istream is(&buffer);
            std::string line;
            std::getline(is, line);
            if (!line.empty()) {
                json j = json::parse(line);
                {
                    std::lock_guard<std::mutex> lock(readMutex_);
                    readQueue_.push(j);
                }
                readCv_.notify_one();
            }
        }
    } catch (std::exception& e) {
        disconnected_ = true;
    }
}

void Bridge::writeLoop() {
    try {
        while (!disconnected_) {
            std::unique_lock<std::mutex> lock(writeMutex_);
            writeCv_.wait(lock, [this] { return !writeQueue_.empty() || disconnected_; });
            while (!writeQueue_.empty()) {
                std::string msg = writeQueue_.front();
                writeQueue_.pop();
                lock.unlock();
                boost::asio::write(socket_, boost::asio::buffer(msg));
                lock.lock();
            }
        }
    } catch (std::exception& e) {
        disconnected_ = true;
    }
}
