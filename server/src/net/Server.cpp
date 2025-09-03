#include "Server.h"

#include "Config.h"
#include "Logger.h"

#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port) : sock_fd(socket(AF_INET, SOCK_STREAM, 0))
{
	if (sock_fd < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not create socket.");
		std::runtime_error("Socket is mandatory to work.");
	}

	int opt = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not set socket option.");
		close(sock_fd);
		std::runtime_error("Socket is mandatory to work.");
	}

	struct sockaddr_in addr
	{
	};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	if (bind(sock_fd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not bind on socket.");
		close(sock_fd);
		std::runtime_error("Socket is mandatory to work.");
	}

	if (listen(sock_fd, SOMAXCONN) < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not listen on socket.");
		close(sock_fd);
		std::runtime_error("Socket is mandatory to work.");
	}

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
 * acceptClient() - accept a client for a tcp connection
 * @param name Out parameter to write the address of the connection into.
 *
 * @return client_fd Successful accepting a client connection.
 * @return -1  On error.
 */
int Server::acceptClient(std::string &name) const
{
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(sock_fd, &rfds);

	timeval tv{};
	unsigned int ms = Config::server().clientConnectTimeoutMs;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = static_cast<suseconds_t>((ms % 1000) * 1000);

	int sel = select(sock_fd + 1, &rfds, nullptr, nullptr, &tv);
	if (sel < 0)
	{
		Logger::Log(LogLevel::ERROR, "select() failed while waiting for client.");
		return -1;
	}
	if (sel == 0)
	{
		errno = EAGAIN;
		Logger::Log(LogLevel::WARNING, "Timed out waiting for client connection.");
		return -1;
	}

	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	int client_fd = accept(sock_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);

	if (client_fd < 0)
	{
		Logger::Log(LogLevel::ERROR, "Could not accept client.");
		return -1;
	}

	name = std::string(inet_ntoa(client_addr.sin_addr));
	return client_fd;
}
