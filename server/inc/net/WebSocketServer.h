#ifndef WEB_SOCKET_SERVER_H
#define WEB_SOCKET_SERVER_H

#include <cstdint>
#include <memory>
#include <string>

class WebSocketServer
{
  public:
	explicit WebSocketServer(std::string route);
	~WebSocketServer();

	WebSocketServer(const WebSocketServer &) = delete;
	WebSocketServer &operator=(const WebSocketServer &) = delete;

	void start(uint16_t port);
	void setInitialMessage(std::string message);
	void publish(std::string message);
	void publish(std::string message, std::string updatedInitialMessage);

  private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

#endif // WEB_SOCKET_SERVER_H
