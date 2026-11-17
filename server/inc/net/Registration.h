#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "Bridge.h"
#include "Server.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct RegisteredClient
{
	std::unique_ptr<Bridge> bridge; // not started yet, so a waiting client cannot grow server-side buffers
	std::string name;				// client-provided, not sanitized
};

struct Listener
{
	std::unique_ptr<Server> server;
	int port;
	std::optional<unsigned int> teamId; // the only team allowed to log in here, any expected team if empty
};

/**
 * Opens one listener per team on its own port, or a single shared one on the default port if teamPorts is empty.
 * Throws std::runtime_error if any port cannot be opened, listeners opened before it are closed again.
 */
std::vector<Listener> openListeners(const std::map<unsigned int, int> &teamPorts);

/**
 * Accepts connections until every expected team has logged in or clientConnectTimeoutMs has passed, whichever
 * comes first. Pending logins are handled concurrently, so a silent or slow peer only ever costs its own slot.
 *
 * @return The validated clients by team ID. Teams that did not log in in time are missing.
 */
std::unordered_map<unsigned int, RegisteredClient> registerClients(const std::vector<Listener> &listeners,
																   const std::vector<unsigned int> &expectedTeamIds);

#endif // REGISTRATION_H
