#include "LiveReplayServer.h"

#include "Logger.h"

#include <arpa/inet.h>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
uint32_t rotateLeft(uint32_t value, unsigned int bits)
{
	return (value << bits) | (value >> (32 - bits));
}

std::array<unsigned char, 20> sha1(const std::string &input)
{
	std::vector<unsigned char> message(input.begin(), input.end());
	const uint64_t bitLength = static_cast<uint64_t>(message.size()) * 8;
	message.push_back(0x80);
	while ((message.size() % 64) != 56)
		message.push_back(0);
	for (int i = 7; i >= 0; --i)
		message.push_back(static_cast<unsigned char>((bitLength >> (i * 8)) & 0xff));

	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xefcdab89;
	uint32_t h2 = 0x98badcfe;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xc3d2e1f0;

	for (size_t chunk = 0; chunk < message.size(); chunk += 64)
	{
		uint32_t words[80]{};
		for (size_t i = 0; i < 16; ++i)
		{
			const size_t offset = chunk + i * 4;
			words[i] = (static_cast<uint32_t>(message[offset]) << 24) |
					   (static_cast<uint32_t>(message[offset + 1]) << 16) |
					   (static_cast<uint32_t>(message[offset + 2]) << 8) |
					   static_cast<uint32_t>(message[offset + 3]);
		}
		for (size_t i = 16; i < 80; ++i)
			words[i] = rotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);

		uint32_t a = h0;
		uint32_t b = h1;
		uint32_t c = h2;
		uint32_t d = h3;
		uint32_t e = h4;
		for (size_t i = 0; i < 80; ++i)
		{
			uint32_t f;
			uint32_t k;
			if (i < 20)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5a827999;
			}
			else if (i < 40)
			{
				f = b ^ c ^ d;
				k = 0x6ed9eba1;
			}
			else if (i < 60)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8f1bbcdc;
			}
			else
			{
				f = b ^ c ^ d;
				k = 0xca62c1d6;
			}
			const uint32_t temp = rotateLeft(a, 5) + f + e + k + words[i];
			e = d;
			d = c;
			c = rotateLeft(b, 30);
			b = a;
			a = temp;
		}
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	std::array<unsigned char, 20> digest{};
	const uint32_t hashes[] = {h0, h1, h2, h3, h4};
	for (size_t i = 0; i < 5; ++i)
		for (size_t j = 0; j < 4; ++j)
			digest[i * 4 + j] = static_cast<unsigned char>((hashes[i] >> (24 - j * 8)) & 0xff);
	return digest;
}

std::string base64(const unsigned char *data, size_t length)
{
	static constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	std::string result;
	for (size_t i = 0; i < length; i += 3)
	{
		const uint32_t value = (static_cast<uint32_t>(data[i]) << 16) |
						   (i + 1 < length ? static_cast<uint32_t>(data[i + 1]) << 8 : 0) |
						   (i + 2 < length ? static_cast<uint32_t>(data[i + 2]) : 0);
		result.push_back(alphabet[(value >> 18) & 0x3f]);
		result.push_back(alphabet[(value >> 12) & 0x3f]);
		result.push_back(i + 1 < length ? alphabet[(value >> 6) & 0x3f] : '=');
		result.push_back(i + 2 < length ? alphabet[value & 0x3f] : '=');
	}
	return result;
}

std::string websocketAcceptKey(const std::string &key)
{
	const auto digest = sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	return base64(digest.data(), digest.size());
}

std::string headerValue(const std::string &request, const std::string &header)
{
	std::istringstream lines(request);
	std::string line;
	while (std::getline(lines, line))
	{
		if (!line.empty() && line.back() == '\r') line.pop_back();
		const size_t colon = line.find(':');
		if (colon == std::string::npos || colon != header.size()) continue;
		bool matches = true;
		for (size_t i = 0; i < header.size(); ++i)
		{
			if (std::tolower(static_cast<unsigned char>(line[i])) !=
				std::tolower(static_cast<unsigned char>(header[i])))
			{
				matches = false;
				break;
			}
		}
		if (!matches) continue;
		size_t start = colon + 1;
		while (start < line.size() && line[start] == ' ')
			++start;
		return line.substr(start);
	}
	return {};
}

bool sendAll(int fd, const unsigned char *data, size_t length)
{
	while (length > 0)
	{
#ifdef MSG_NOSIGNAL
		const ssize_t sent = send(fd, data, length, MSG_NOSIGNAL);
#else
		const ssize_t sent = send(fd, data, length, 0);
#endif
		if (sent <= 0) return false;
		data += sent;
		length -= static_cast<size_t>(sent);
	}
	return true;
}
} // namespace

LiveReplayServer::~LiveReplayServer()
{
	running_ = false;
	if (listenFd_ >= 0)
	{
		shutdown(listenFd_, SHUT_RDWR);
		close(listenFd_);
		listenFd_ = -1;
	}
	if (acceptThread_.joinable()) acceptThread_.join();
	std::lock_guard<std::mutex> lock(mutex_);
	for (int fd : clients_)
		close(fd);
}

bool LiveReplayServer::start(unsigned int port, const json &initialReplay)
{
	if (port == 0 || running_) return false;
	listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (listenFd_ < 0) return false;
	int enabled = 1;
	setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));

	sockaddr_in address{};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(static_cast<uint16_t>(port));
	if (bind(listenFd_, reinterpret_cast<sockaddr *>(&address), sizeof(address)) < 0 ||
		listen(listenFd_, SOMAXCONN) < 0)
	{
		close(listenFd_);
		listenFd_ = -1;
		return false;
	}

	replay_ = initialReplay;
	running_ = true;
	acceptThread_ = std::thread(&LiveReplayServer::acceptLoop, this);
	Logger::Log("Live replay WebSocket listening on port " + std::to_string(port) + "...");
	return true;
}

void LiveReplayServer::acceptLoop()
{
	while (running_)
	{
		const int clientFd = accept(listenFd_, nullptr, nullptr);
		if (clientFd < 0)
		{
			if (running_) Logger::Log(LogLevel::WARNING, "Could not accept live replay WebSocket client.");
			continue;
		}

		std::string request;
		std::array<char, 2048> buffer{};
		while (request.find("\r\n\r\n") == std::string::npos && request.size() < 8192)
		{
			const ssize_t received = recv(clientFd, buffer.data(), buffer.size(), 0);
			if (received <= 0) break;
			request.append(buffer.data(), static_cast<size_t>(received));
		}
		const std::string key = headerValue(request, "Sec-WebSocket-Key");
		if (key.empty())
		{
			close(clientFd);
			continue;
		}

		const std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
								 "Upgrade: websocket\r\n"
								 "Connection: Upgrade\r\n"
								 "Sec-WebSocket-Accept: " +
							 websocketAcceptKey(key) + "\r\n\r\n";
		if (!sendAll(clientFd, reinterpret_cast<const unsigned char *>(response.data()), response.size()))
		{
			close(clientFd);
			continue;
		}

		std::lock_guard<std::mutex> lock(mutex_);
		json message = {{"type", "snapshot"}, {"replay", replay_}};
		if (sendFrame(clientFd, message.dump()))
		{
			const int flags = fcntl(clientFd, F_GETFL, 0);
			if (flags >= 0) fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
			clients_.push_back(clientFd);
		}
		else
			close(clientFd);
	}
}

bool LiveReplayServer::sendFrame(int clientFd, const std::string &payload) const
{
	std::vector<unsigned char> frame;
	frame.reserve(payload.size() + 10);
	frame.push_back(0x81);
	if (payload.size() <= 125)
	{
		frame.push_back(static_cast<unsigned char>(payload.size()));
	}
	else if (payload.size() <= 65535)
	{
		frame.push_back(126);
		frame.push_back(static_cast<unsigned char>((payload.size() >> 8) & 0xff));
		frame.push_back(static_cast<unsigned char>(payload.size() & 0xff));
	}
	else
	{
		frame.push_back(127);
		for (int shift = 56; shift >= 0; shift -= 8)
			frame.push_back(static_cast<unsigned char>((static_cast<uint64_t>(payload.size()) >> shift) & 0xff));
	}
	frame.insert(frame.end(), payload.begin(), payload.end());
	return sendAll(clientFd, frame.data(), frame.size());
}

void LiveReplayServer::broadcast(const std::string &message)
{
	std::lock_guard<std::mutex> lock(mutex_);
	for (auto it = clients_.begin(); it != clients_.end();)
	{
		if (sendFrame(*it, message))
		{
			++it;
		}
		else
		{
			close(*it);
			it = clients_.erase(it);
		}
	}
}

void LiveReplayServer::publishTick(unsigned long long tick, const json &tickData)
{
	if (!running_) return;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (!tickData.empty())
		{
			if (!replay_["ticks"].is_object()) replay_["ticks"] = json::object();
			replay_["ticks"][std::to_string(tick)] = tickData;
		}
		replay_["full_tick_amount"] = tick;
	}
	json message = {{"type", "tick"}, {"tick", tick}, {"data", tickData}};
	broadcast(message.dump());
}

void LiveReplayServer::publishComplete(const json &replay)
{
	if (!running_) return;
	{
		std::lock_guard<std::mutex> lock(mutex_);
		replay_ = replay;
	}
	json message = {{"type", "complete"}, {"replay", replay}};
	broadcast(message.dump());
}
