#ifndef LIVE_REPLAY_SERVER_H
#define LIVE_REPLAY_SERVER_H

#include "json.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using json = nlohmann::ordered_json;

class LiveReplayServer
{
  public:
	LiveReplayServer() = default;
	~LiveReplayServer();

	LiveReplayServer(const LiveReplayServer &) = delete;
	LiveReplayServer &operator=(const LiveReplayServer &) = delete;

	bool start(unsigned int port, const json &initialReplay);
	void publishTick(unsigned long long tick, const json &tickData);
	void publishComplete(const json &replay);
	bool isRunning() const { return running_; }

  private:
	void acceptLoop();
	void broadcast(const std::string &message);
	bool sendFrame(int clientFd, const std::string &payload) const;

	std::atomic<bool> running_{false};
	int listenFd_ = -1;
	std::thread acceptThread_;
	mutable std::mutex mutex_;
	std::vector<int> clients_;
	json replay_ = json::object();
};

#endif // LIVE_REPLAY_SERVER_H
