#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "Bridge.h"
#include "Server.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct RegisteredClient
{
	std::unique_ptr<Bridge> bridge; // not started yet, so a waiting client cannot grow server-side buffers
	std::string name;				// client-provided, not sanitized
};

/**
 * Accepts connections until every expected team has logged in or clientConnectTimeoutMs has passed, whichever
 * comes first. Pending logins are handled concurrently, so a silent or slow peer only ever costs its own slot.
 *
 * @return The validated clients by team ID. Teams that did not log in in time are missing.
 */
std::unordered_map<unsigned int, RegisteredClient> registerClients(const Server &server,
																   const std::vector<unsigned int> &expectedTeamIds);

#endif // REGISTRATION_H
