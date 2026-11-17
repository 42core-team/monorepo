#include "Bridge.h"

#include "Board.h"

#include <chrono>
#include <cstring> // For strerror()
#include <errno.h>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <sys/socket.h> // For shutdown()
#include <unistd.h>		// For close(), read() and write()
#ifdef __APPLE__
#	include <sys/event.h>
#endif

static constexpr std::chrono::milliseconds WRITE_FLUSH_TIMEOUT{200};
// the game takes one message per tick, so a client sending faster only fills its own socket buffer
static constexpr size_t MAX_QUEUED_MESSAGES = 8;
// legitimate packets nest about 5 deep, deeper ones would overflow the stack in nlohmann's recursive copy and dump
static constexpr size_t MAX_JSON_DEPTH = 32;
// a reading client lags a message or two behind, so a backlog this large means it stopped reading
static constexpr size_t MAX_QUEUED_WRITE_BYTES = 8 * 1024 * 1024;

static bool exceedsJsonDepth(const std::string &line, size_t maxDepth)
{
	size_t depth = 0;
	bool inString = false;
	bool escaped = false;
	for (char c : line)
	{
		if (inString)
		{
			if (escaped)
				escaped = false;
			else if (c == '\\')
				escaped = true;
			else if (c == '"')
				inString = false;
		}
		else if (c == '"')
			inString = true;
		else if (c == '{' || c == '[')
		{
			if (++depth > maxDepth) return true;
		}
		else if ((c == '}' || c == ']') && depth > 0)
			depth--;
	}
	return false;
}

Bridge::Bridge(int socket_fd, unsigned int teamId) : socket_fd_(socket_fd), team_id_(teamId), disconnected_(false)
{
	// Set a timeout for the socket.
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
	{
		Logger::Log(LogLevel::WARNING, "Error setting socket timeout: " + std::string(strerror(errno)) +
											   ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
	// Not every platform wakes a blocked send on shutdown(), so make it return regularly to check abortWrites_.
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	if (setsockopt(socket_fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
	{
		Logger::Log(LogLevel::WARNING, "Error setting socket timeout: " + std::string(strerror(errno)) +
											   ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
}

Bridge::~Bridge()
{
	{
		// under the lock, or the writer could check it right before waiting and sleep through the notify below
		std::lock_guard<std::mutex> lock(writeMutex_);
		disconnected_ = true;
	}

	auto core = Board::instance().getCoreByTeamId(team_id_);
	if (core != nullptr)
	{
		Board::instance().removeObjectById(core->getId());
		Logger::Log("Core of team " + std::to_string(team_id_) + " has been removed from the board.");
	}
	else
	{
		Logger::Log(LogLevel::WARNING,
					"Core not found for team ID: " + std::to_string(team_id_) + ". Unable to remove object.");
	}

	{
		// disconnected_ was set under writeMutex_ only, so make sure a reader between its check and wait sees it
		std::lock_guard<std::mutex> lock(readMutex_);
	}
	writeCv_.notify_all();
	readCv_.notify_all();
	readSpaceCv_.notify_all();
	// wake a blocked read, but give queued messages like the final state a bounded chance to reach the peer
	shutdown(socket_fd_, SHUT_RD);
	if (readThread_.joinable()) readThread_.join();
	if (writeThread_.joinable())
	{
		std::unique_lock<std::mutex> lock(writeMutex_);
		writeCv_.wait_for(lock, WRITE_FLUSH_TIMEOUT, [this] { return writerDone_; });
	}
	abortWrites_ = true;
	shutdown(socket_fd_, SHUT_RDWR); // wakes a write blocked on a peer that stopped reading
	if (writeThread_.joinable()) writeThread_.join();
	close(socket_fd_);
}

void Bridge::start()
{
	readThread_ = std::thread(&Bridge::readLoop, this);
	writeThread_ = std::thread(&Bridge::writeLoop, this);
}

void Bridge::sendMessage(const json &message)
{
	if (disconnected_) return;
	std::string msg = message.dump() + "\n";
	bool overflow = false;
	{
		std::lock_guard<std::mutex> lock(writeMutex_);
		// bounded by bytes since the error strings make the message size attacker-influenced
		if (!writeQueue_.empty() && writeQueueBytes_ + msg.size() > MAX_QUEUED_WRITE_BYTES)
		{
			overflow = true;
			disconnected_ = true;
			writeQueue_ = {};
			writeQueueBytes_ = 0;
		}
		else
		{
			writeQueueBytes_ += msg.size();
			writeQueue_.push(std::move(msg));
		}
	}
	if (overflow)
	{
		Logger::LogWarn("Team " + std::to_string(team_id_) + " is not reading its messages. Disconnecting.");
		{
			// like in the destructor, so a reader between its check and wait sees disconnected_
			std::lock_guard<std::mutex> lock(readMutex_);
		}
		readCv_.notify_all();
		readSpaceCv_.notify_all();
	}
	writeCv_.notify_one();
}

bool Bridge::receiveMessage(json &message)
{
	std::unique_lock<std::mutex> lock(readMutex_);
	if (readQueue_.empty())
	{
		readCv_.wait(lock, [this] { return !readQueue_.empty() || disconnected_; });
	}
	if (readQueue_.empty()) return false;
	message = readQueue_.front();
	readQueue_.pop();
	readSpaceCv_.notify_one();
	// std::cout << "Server received message: " << message << std::endl;
	return true;
}
bool Bridge::tryReceiveMessage(json &message)
{
	std::lock_guard<std::mutex> lock(readMutex_);
	if (readQueue_.empty()) return false;
	message = readQueue_.front();
	readQueue_.pop();
	readSpaceCv_.notify_one();
	return true;
}

bool Bridge::isDisconnected()
{
	return disconnected_;
}

void Bridge::detectPeerClosed()
{
	// unlike a peek these also see a close behind data the peer sent after its login
#ifdef POLLRDHUP
	pollfd pfd{socket_fd_, POLLRDHUP, 0};
	if (poll(&pfd, 1, 0) > 0 && (pfd.revents & (POLLRDHUP | POLLHUP | POLLERR | POLLNVAL)) != 0) disconnected_ = true;
#elif defined(__APPLE__)
	const int kq = kqueue();
	if (kq < 0)
	{
		Logger::LogWarn("kqueue failed: " + std::string(strerror(errno)) + ". Not checking team " +
						std::to_string(team_id_) + " before the game.");
		return;
	}
	struct kevent change;
	struct kevent event;
	EV_SET(&change, socket_fd_, EVFILT_READ, EV_ADD, 0, 0, nullptr);
	const timespec noWait{0, 0};
	if (kevent(kq, &change, 1, &event, 1, &noWait) > 0 && (event.flags & (EV_EOF | EV_ERROR)) != 0)
		disconnected_ = true;
	close(kq);
#else
	char byte;
	const ssize_t n = recv(socket_fd_, &byte, 1, MSG_PEEK | MSG_DONTWAIT);
	if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) disconnected_ = true;
#endif
}

void Bridge::readLoop()
{
	try
	{
		constexpr size_t buffer_size = 1024;
		const size_t MAX_MESSAGE_BYTES = Config::server().clientPacketsMaxSizeKb * 1024;
		char buffer[buffer_size];
		std::string data;
		while (!disconnected_)
		{
			ssize_t n = ::read(socket_fd_, buffer, buffer_size);
			if (n < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
				}
				if (!disconnected_)
					Logger::Log(LogLevel::WARNING, "Read error: " + std::string(strerror(errno)) + ". Disconnecting " +
														   std::to_string(team_id_) + ".");
				disconnected_ = true;
				readCv_.notify_all();
				writeCv_.notify_all();
				break;
			}
			else if (n == 0)
			{
				if (!disconnected_)
					Logger::Log(LogLevel::WARNING,
								"Connection closed by peer. Disconnecting " + std::to_string(team_id_) + ".");
				disconnected_ = true;
				readCv_.notify_all();
				writeCv_.notify_all();
				break;
			}
			data.append(buffer, n);

			// message size limit
			if (data.find('\n') == std::string::npos && data.size() > MAX_MESSAGE_BYTES)
			{
				Logger::LogWarn("Incoming message too large from team " + std::to_string(team_id_) + " (>" +
								std::to_string(MAX_MESSAGE_BYTES) + " bytes). Disconnecting.");
				// the game thread records the death reason, Board and ReplayEncoder are not thread-safe
				spamming_ = true;
				disconnected_ = true;
				readCv_.notify_all();
				writeCv_.notify_all();
				break;
			}

			size_t pos;
			while ((pos = data.find('\n')) != std::string::npos)
			{
				std::string line = data.substr(0, pos);
				data.erase(0, pos + 1);
				if (!line.empty())
				{
					if (exceedsJsonDepth(line, MAX_JSON_DEPTH))
					{
						Logger::LogWarn("JSON nesting deeper than " + std::to_string(MAX_JSON_DEPTH) +
										" levels. Skipping message (team id: " + std::to_string(team_id_) + ").");
						continue;
					}
					try
					{
						json j = json::parse(line);
						{
							std::unique_lock<std::mutex> lock(readMutex_);
							readSpaceCv_.wait(lock, [this]
											  { return readQueue_.size() < MAX_QUEUED_MESSAGES || disconnected_; });
							if (readQueue_.size() >= MAX_QUEUED_MESSAGES) break; // disconnected while full
							readQueue_.push(std::move(j));
						}
						readCv_.notify_one();
					}
					catch (json::parse_error &e)
					{
						Logger::Log(LogLevel::WARNING,
									"JSON parse error: " + std::string(e.what()) +
											". Skipping message (team id: " + std::to_string(team_id_) + ").");
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::WARNING,
					"Exception: " + std::string(e.what()) + ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
		readCv_.notify_all();
		writeCv_.notify_all();
	}
}

void Bridge::writeLoop()
{
	try
	{
		for (;;)
		{
			std::unique_lock<std::mutex> lock(writeMutex_);
			writeCv_.wait(lock, [this] { return !writeQueue_.empty() || disconnected_; });
			while (!writeQueue_.empty())
			{
				std::string msg = std::move(writeQueue_.front());
				writeQueue_.pop();
				writeQueueBytes_ -= msg.size();
				lock.unlock();
				bool sendFailed = false;
				const char *data = msg.c_str();
				size_t remaining = msg.size();
				while (remaining > 0)
				{
					ssize_t n = ::send(socket_fd_, data, remaining, MSG_NOSIGNAL);
					if (n < 0)
					{
						if ((errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) && !abortWrites_) continue;
						Logger::Log(LogLevel::WARNING, std::string("Write error: ") + std::string(strerror(errno)) +
															   ". Disconnecting " + std::to_string(team_id_) + ".");
						disconnected_ = true;
						sendFailed = true;
						break;
					}
					remaining -= n;
					data += n;
				}
				lock.lock();
				if (sendFailed)
				{
					writeQueue_ = {}; // the socket is unusable, don't fail on every remaining message
					writeQueueBytes_ = 0;
				}
			}
			if (disconnected_ && writeQueue_.empty()) break;
		}
	}
	catch (std::exception &e)
	{
		Logger::Log(LogLevel::WARNING,
					"Exception: " + std::string(e.what()) + ". Disconnecting " + std::to_string(team_id_) + ".");
		disconnected_ = true;
	}
	{
		std::lock_guard<std::mutex> lock(writeMutex_);
		writerDone_ = true;
	}
	writeCv_.notify_all();
}
