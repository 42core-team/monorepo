#include "Registration.h"

#include "Config.h"
#include "Logger.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <json-schema.hpp>
#include <limits>
#include <list>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
using nlohmann::json_schema::json_validator;

using Clock = std::chrono::steady_clock;

namespace
{
constexpr size_t MAX_PENDING_LOGINS = 64;
constexpr size_t MAX_LOGIN_BYTES = 4096;
constexpr std::chrono::milliseconds ACCEPT_RETRY_DELAY{100};

struct PendingLogin
{
	int fd;
	std::string addr;
	Clock::time_point deadline; // fixed on accept, traffic never extends it
	const Listener &listener;	// the connection arrived here
	std::string buffer;

	PendingLogin(int fd, std::string addr, Clock::time_point deadline, const Listener &listener)
		: fd(fd), addr(std::move(addr)), deadline(deadline), listener(listener)
	{
	}
	~PendingLogin()
	{
		if (fd >= 0) close(fd);
	}
	PendingLogin(const PendingLogin &) = delete;
	PendingLogin &operator=(const PendingLogin &) = delete;
};

enum class ReadResult
{
	WAITING,
	COMPLETE,
	FAILED
};

// Only consumes bytes up to the login's newline, anything sent after it stays in the socket for the Bridge.
ReadResult readLogin(PendingLogin &login)
{
	char chunk[MAX_LOGIN_BYTES + 1];
	const size_t room = sizeof(chunk) - login.buffer.size();
	const ssize_t peeked = recv(login.fd, chunk, room, MSG_PEEK | MSG_DONTWAIT);
	if (peeked < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) return ReadResult::WAITING;
		Logger::LogWarn("Read error during login from " + login.addr + ": " + std::string(strerror(errno)) + ".");
		return ReadResult::FAILED;
	}
	if (peeked == 0)
	{
		Logger::LogWarn("Connection from " + login.addr + " closed before completing a login.");
		return ReadResult::FAILED;
	}

	const char *newline = static_cast<const char *>(memchr(chunk, '\n', peeked));
	const size_t lineBytes = newline ? static_cast<size_t>(newline - chunk) : static_cast<size_t>(peeked);
	const size_t consume = newline ? lineBytes + 1 : lineBytes;
	if (recv(login.fd, chunk, consume, MSG_DONTWAIT) != static_cast<ssize_t>(consume))
	{
		Logger::LogWarn("Read error during login from " + login.addr + ".");
		return ReadResult::FAILED;
	}
	login.buffer.append(chunk, lineBytes);
	if (login.buffer.size() > MAX_LOGIN_BYTES)
	{
		Logger::LogWarn("Login message from " + login.addr + " is too large.");
		return ReadResult::FAILED;
	}
	return newline ? ReadResult::COMPLETE : ReadResult::WAITING;
}

json_validator loadLoginValidator()
{
	json_validator validator;
	try
	{
		validator.set_root_schema(Config::load_json_schema("packets/login-message.schema.json"));
	}
	catch (const std::exception &e)
	{
		Logger::LogErr("Invalid login message schema: " + std::string(e.what()));
		exit(EXIT_FAILURE);
	}
	return validator;
}

// Touches no player state until the login is fully validated.
void handleLogin(PendingLogin &login, const json_validator &validator, const std::vector<unsigned int> &expectedTeamIds,
				 std::unordered_map<unsigned int, RegisteredClient> &clients)
{
	unsigned int teamId = 0;
	std::string name;
	try
	{
		const json message = json::parse(login.buffer);
		validator.validate(message);
		teamId = message["id"].get<unsigned int>();
		name = message["name"].get<std::string>();
	}
	catch (const std::exception &e)
	{
		Logger::LogWarn("Invalid login message from " + login.addr + ": " + e.what());
		return;
	}

	if (std::find(expectedTeamIds.begin(), expectedTeamIds.end(), teamId) == expectedTeamIds.end())
	{
		Logger::LogWarn("Received unexpected team ID " + std::to_string(teamId) + " from " + login.addr + ".");
		return;
	}
	if (login.listener.teamId && *login.listener.teamId != teamId)
	{
		Logger::LogWarn("Received team ID " + std::to_string(teamId) + " on port " +
						std::to_string(login.listener.port) + ", which is reserved for team " +
						std::to_string(*login.listener.teamId) + ", from " + login.addr + ".");
		return;
	}
	if (clients.find(teamId) != clients.end())
	{
		Logger::LogWarn("Duplicate connection for team ID " + std::to_string(teamId) + " from " + login.addr + ".");
		return;
	}

	clients[teamId] = RegisteredClient{std::make_unique<Bridge>(login.fd, teamId), std::move(name)};
	login.fd = -1; // now owned by the Bridge
	Logger::Log("Accepted team " + std::to_string(teamId));
}

int pollTimeoutMs(Clock::time_point now, Clock::time_point wakeUp)
{
	if (wakeUp <= now) return 0;
	// a negative timeout would make poll wait forever, the loop rechecks the deadline after a clamped wait
	const auto ms = std::chrono::ceil<std::chrono::milliseconds>(wakeUp - now).count();
	return static_cast<int>(std::min<long long>(ms, std::numeric_limits<int>::max()));
}
} // namespace

std::vector<Listener> openListeners(const std::map<unsigned int, int> &teamPorts)
{
	std::vector<Listener> listeners;
	if (teamPorts.empty())
	{
		listeners.push_back({std::make_unique<Server>(Server::DEFAULT_PORT), Server::DEFAULT_PORT, std::nullopt});
		return listeners;
	}
	for (const auto &[teamId, port] : teamPorts)
	{
		listeners.push_back({std::make_unique<Server>(port), port, teamId});
		Logger::Log("Port " + std::to_string(port) + " only accepts team " + std::to_string(teamId) + ".");
	}
	return listeners;
}

std::unordered_map<unsigned int, RegisteredClient> registerClients(const std::vector<Listener> &listeners,
																   const std::vector<unsigned int> &expectedTeamIds)
{
	const json_validator validator = loadLoginValidator();
	const std::chrono::milliseconds loginTimeout{Config::server().clientLoginTimeoutMs};
	const Clock::time_point registrationDeadline =
			Clock::now() + std::chrono::milliseconds(Config::server().clientConnectTimeoutMs);

	std::unordered_map<unsigned int, RegisteredClient> clients;
	std::list<PendingLogin> pending; // oldest first, all share one timeout so also ordered by deadline
	Clock::time_point acceptAllowedAt = Clock::now();

	while (clients.size() < expectedTeamIds.size())
	{
		Clock::time_point now = Clock::now();
		if (now >= registrationDeadline)
		{
			Logger::LogWarn("Registration deadline reached with " + std::to_string(clients.size()) + " / " +
							std::to_string(expectedTeamIds.size()) + " teams logged in.");
			break;
		}
		while (!pending.empty() && pending.front().deadline <= now)
		{
			Logger::LogWarn("Login from " + pending.front().addr + " timed out.");
			pending.pop_front();
		}

		std::vector<pollfd> fds;
		fds.reserve(listeners.size() + pending.size());
		const bool acceptAllowed = now >= acceptAllowedAt;
		for (const Listener &listener : listeners)
			fds.push_back({acceptAllowed ? listener.server->getSock() : -1, POLLIN, 0}); // poll skips negative fds
		for (const PendingLogin &login : pending)
			fds.push_back({login.fd, POLLIN, 0});

		Clock::time_point wakeUp = registrationDeadline;
		if (!pending.empty()) wakeUp = std::min(wakeUp, pending.front().deadline);
		if (!acceptAllowed) wakeUp = std::min(wakeUp, acceptAllowedAt);

		if (poll(fds.data(), fds.size(), pollTimeoutMs(now, wakeUp)) < 0)
		{
			if (errno == EINTR) continue;
			Logger::LogErr("poll() failed during registration: " + std::string(strerror(errno)) + ".");
			break;
		}

		// poll may have returned long after it was ready, the top of the loop handles anything expired since
		now = Clock::now();
		if (now >= registrationDeadline) continue;

		size_t fdIndex = listeners.size();
		for (auto it = pending.begin(); it != pending.end() && clients.size() < expectedTeamIds.size(); ++fdIndex)
		{
			if (fds[fdIndex].revents == 0 || it->deadline <= now)
			{
				++it;
				continue;
			}
			const ReadResult result = readLogin(*it);
			if (result == ReadResult::WAITING)
			{
				++it;
				continue;
			}
			if (result == ReadResult::COMPLETE) handleLogin(*it, validator, expectedTeamIds, clients);
			it = pending.erase(it);
		}

		for (size_t i = 0; i < listeners.size() && clients.size() < expectedTeamIds.size(); ++i)
		{
			if (fds[i].revents == 0) continue;
			const Listener &listener = listeners[i];

			std::string addr;
			const int clientFd = listener.server->acceptClient(addr);
			now = Clock::now();
			if (clientFd < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || errno == ECONNABORTED) continue;
				// the listener stays readable, so back off instead of spinning on e.g. EMFILE
				Logger::LogWarn("accept failed: " + std::string(strerror(errno)));
				acceptAllowedAt = now + ACCEPT_RETRY_DELAY;
				continue;
			}
			Logger::Log("Accepted connection from " + addr + " on port " + std::to_string(listener.port));
			// bounded per listener, so connections on one team's port can never evict a login on another's
			const auto onListener = [&listener](const PendingLogin &login) { return &login.listener == &listener; };
			if (static_cast<size_t>(std::count_if(pending.begin(), pending.end(), onListener)) >= MAX_PENDING_LOGINS)
			{
				// a healthy client logs in right after connecting, so idle connections cannot lock it out this way
				const auto oldest = std::find_if(pending.begin(), pending.end(), onListener);
				Logger::LogWarn("Too many pending logins, dropping oldest from " + oldest->addr + ".");
				pending.erase(oldest);
			}
			pending.emplace_back(clientFd, addr, now + loginTimeout, listener);
		}
	}
	return clients;
}
