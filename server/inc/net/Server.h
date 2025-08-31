#ifndef SERVER_H
#define SERVER_H

#include <string>

class Server {

  const int sock_fd;

 public:
  // Constructor
  explicit Server(int port = 4444);
  // Deconstructor
  ~Server();

  // Delete all other, because we only want a single server
  Server(const Server& other) = delete;
  Server(Server&& other) = delete;
  Server& operator=(const Server& other) = delete;
  Server& operator=(Server&& other) = delete;

  int getSock() const;
  int acceptClient(std::string& name) const;
};

#endif
