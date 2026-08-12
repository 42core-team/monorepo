#define CROW_ENFORCE_WS_SPEC
#include "WebSocketServer.h"

#include "Logger.h"
#include "crow.h"

#include <mutex>
#include <optional>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

struct WebSocketServer::Impl
{
	crow::SimpleApp app;
	// ponytail: One lock orders joins and broadcasts. Shard only if fan-out becomes a measured bottleneck.
	std::mutex mutex;
	std::unordered_set<crow::websocket::connection *> clients;
	std::vector<std::string> retainedMessages;
	std::thread serverThread;

	explicit Impl(const std::string &route)
	{
		app.route_dynamic(route)
				.websocket(&app)
				.onopen(
						[this](crow::websocket::connection &connection)
						{
							std::lock_guard<std::mutex> lock(mutex);
							clients.insert(&connection);
							for (const std::string &message : retainedMessages)
								connection.send_text(message);
						})
				.onclose(
						[this](crow::websocket::connection &connection, const std::string &, uint16_t)
						{
							std::lock_guard<std::mutex> lock(mutex);
							clients.erase(&connection);
						})
				.onerror(
						[this](crow::websocket::connection &connection, const std::string &)
						{
							std::lock_guard<std::mutex> lock(mutex);
							clients.erase(&connection);
						});
	}

	~Impl()
	{
		app.stop();
		if (serverThread.joinable()) serverThread.join();
	}

	void broadcast(const std::string &message)
	{
		for (crow::websocket::connection *client : clients)
			client->send_text(message);
	}

	void publish(std::string message, const std::optional<std::string> &updatedInitialMessage)
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (updatedInitialMessage && !retainedMessages.empty()) retainedMessages.front() = *updatedInitialMessage;
		retainedMessages.push_back(message);
		broadcast(message);
	}
};

WebSocketServer::WebSocketServer(std::string route) : impl_(std::make_unique<Impl>(route))
{
}

WebSocketServer::~WebSocketServer() = default;

void WebSocketServer::start(uint16_t port)
{
	if (impl_->serverThread.joinable()) return;
	impl_->serverThread = std::thread(
			[this, port]()
			{
				try
				{
					impl_->app.signal_clear().loglevel(crow::LogLevel::Warning).port(port).run();
				}
				catch (const std::exception &error)
				{
					Logger::LogErr("WebSocket stopped: " + std::string(error.what()));
				}
			});
}

void WebSocketServer::setInitialMessage(std::string message)
{
	std::lock_guard<std::mutex> lock(impl_->mutex);
	impl_->retainedMessages = {message};
	impl_->broadcast(message);
}

void WebSocketServer::publish(std::string message)
{
	impl_->publish(std::move(message), std::nullopt);
}

void WebSocketServer::publish(std::string message, std::string updatedInitialMessage)
{
	impl_->publish(std::move(message), std::move(updatedInitialMessage));
}
