#include "Server.h"

#include "Logger.h"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
// Closes fd if it is open and throws, keeping errno's text so the caller can log why.
[[noreturn]] void failListening(int fd, int port, const std::string &what)
{
	const int savedErrno = errno;
	if (fd >= 0) close(fd);
	throw std::runtime_error(what + " on port " + std::to_string(port) + ": " + strerror(savedErrno));
}
} // namespace

Server::Server(int port) : sock_fd(socket(AF_INET, SOCK_STREAM, 0))
{
	if (sock_fd < 0) failListening(sock_fd, port, "Could not create socket");

	int opt = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		failListening(sock_fd, port, "Could not set socket option");

	struct sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sock_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
		failListening(sock_fd, port, "Could not bind on socket");

	// a client aborting between poll() and accept() must not be able to block the accept
	int flags = fcntl(sock_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		failListening(sock_fd, port, "Could not set socket to non-blocking");

	if (listen(sock_fd, SOMAXCONN) < 0) failListening(sock_fd, port, "Could not listen on socket");

	Logger::Log("Server listening on port " + std::to_string(port) + "...");
}

Server::~Server()
{
	close(sock_fd);
}

/**
 * getSock() - Return the listening socket
 *
 * @return sock_fd
 */
int Server::getSock() const
{
	return sock_fd;
}

/**
 * acceptClient() - accept a pending client for a tcp connection without blocking
 * @param name Out parameter to write the address of the connection into.
 *
 * @return client_fd Successful accepting a client connection, always in blocking mode.
 * @return -1  On error or if no client is pending, errno tells which.
 */
int Server::acceptClient(std::string &name) const
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(sock_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

	if (client_fd < 0) return -1;

	// some platforms inherit O_NONBLOCK from the listening socket
	int flags = fcntl(client_fd, F_GETFL, 0);
	if (flags < 0 || fcntl(client_fd, F_SETFL, flags & ~O_NONBLOCK) < 0)
	{
		int fcntl_errno = errno;
		close(client_fd);
		errno = fcntl_errno;
		return -1;
	}

	name = std::string(inet_ntoa(client_addr.sin_addr));
	return client_fd;
}
