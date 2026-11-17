#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server
{

	const int sock_fd;

  public:
	static constexpr int DEFAULT_PORT = 4444;

	// Constructor, throws std::runtime_error if the port cannot be opened for listening
	explicit Server(int port = DEFAULT_PORT);
	// Deconstructor
	~Server();

	// Delete all other, because each listening socket has a single owner
	Server(const Server &other) = delete;
	Server(Server &&other) = delete;
	Server &operator=(const Server &other) = delete;
	Server &operator=(Server &&other) = delete;

	int getSock() const;
	int acceptClient(std::string &name) const;
};

#endif
