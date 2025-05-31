#include "Bridge.h"

#include <sys/socket.h> // For shutdown()
#include <unistd.h>		// For close(), read() and write()
#include <cstring>		// For strerror()
#include <iostream>
#include <stdexcept>
#include <errno.h>

Bridge::Bridge(int socket_fd, unsigned int teamId)
	: socket_fd_(socket_fd), team_id_(teamId), disconnected_(false)
{
	// Set a timeout for the socket.
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		Logger::Log(LogLevel::WARNING, "Error setting socket timeout: " + std::string(strerror(errno)) + ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
}

Bridge::~Bridge()
{
	disconnected_ = true;

	writeCv_.notify_all();
	readCv_.notify_all();
	shutdown(socket_fd_, SHUT_RDWR);
	if (readThread_.joinable())
		readThread_.join();
	if (writeThread_.joinable())
		writeThread_.join();
	close(socket_fd_);
}

void Bridge::start()
{
	readThread_ = std::thread(&Bridge::readLoop, this);
	writeThread_ = std::thread(&Bridge::writeLoop, this);
}

void Bridge::sendMessage(const json &message)
{
	std::string msg = message.dump() + "\n";
	{
		std::lock_guard<std::mutex> lock(writeMutex_);
		writeQueue_.push(msg);
	}
	writeCv_.notify_one();
}

bool Bridge::receiveMessage(json &message)
{
	std::unique_lock<std::mutex> lock(readMutex_);
	if (readQueue_.empty())
	{
		readCv_.wait(lock, [this]
					 { return !readQueue_.empty() || disconnected_; });
	}
	if (readQueue_.empty())
		return false;
	message = readQueue_.front();
	readQueue_.pop();
	// std::cout << "Server received message: " << message << std::endl;
	return true;
}

bool Bridge::isDisconnected()
{
	return disconnected_;
}

void Bridge::readLoop()
{
	try
	{
		constexpr size_t buffer_size = 1024;
		char buffer[buffer_size];
		std::string data;
		while (!disconnected_)
		{
			ssize_t n = ::read(socket_fd_, buffer, buffer_size);
			if (n <= 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				else
				{
					Logger::Log(LogLevel::WARNING, "Read error: " + std::string(strerror(errno)) + ". Disconnecting " + std::to_string(team_id_) + ".");
					disconnected_ = true;
					break;
				}
			}
			data.append(buffer, n);

			size_t pos;
			while ((pos = data.find('\n')) != std::string::npos)
			{
				std::string line = data.substr(0, pos);
				data.erase(0, pos + 1);
				if (!line.empty())
				{
					try
					{
						json j = json::parse(line);
						{
							std::lock_guard<std::mutex> lock(readMutex_);
							readQueue_.push(j);
						}
						readCv_.notify_one();
					}
					catch (json::parse_error &e)
					{
						Logger::Log(LogLevel::WARNING, "JSON parse error: " + std::string(e.what()) + ". Skipping message (team id: " + std::to_string(team_id_) + ").");
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::WARNING, "Exception: " + std::string(e.what()) + ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
}

void Bridge::writeLoop()
{
	try
	{
		while (!disconnected_)
		{
			std::unique_lock<std::mutex> lock(writeMutex_);
			writeCv_.wait(lock, [this]
						  { return !writeQueue_.empty() || disconnected_; });
			while (!writeQueue_.empty())
			{
				std::string msg = writeQueue_.front();
				writeQueue_.pop();
				lock.unlock();
				const char *data = msg.c_str();
				size_t remaining = msg.size();
				while (remaining > 0)
				{
					ssize_t n = ::write(socket_fd_, data, remaining);
					if (n < 0)
					{
						Logger::Log(LogLevel::WARNING, std::string("Write error: ") + std::string(strerror(errno)) + ". Disconnecting " + std::to_string(team_id_) + ".");
						disconnected_ = true;
						break;
					}
					remaining -= n;
					data += n;
				}
				lock.lock();
			}
		}
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::WARNING, "Exception: " + std::string(e.what()) + ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
}
