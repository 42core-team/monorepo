#ifndef BRIDGE_H
#define BRIDGE_H

#include "Logger.h"
#include "json.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <sys/time.h>
#include <thread>
using json = nlohmann::ordered_json;

class Bridge
{
  public:
	explicit Bridge(int socket_fd, unsigned int teamId);
	~Bridge();

	void sendMessage(const json &message);
	bool receiveMessage(json &message);
	bool tryReceiveMessage(json &message);
	bool isDisconnected();
	void detectPeerClosed(); // for before start(), when no read thread notices a closed connection yet

	void start();

	unsigned int getTeamId() const { return team_id_; }
	bool wasSpamming() const { return spamming_; }

  private:
	void readLoop();
	void writeLoop();

	int socket_fd_;
	unsigned int team_id_;
	std::thread readThread_;
	std::thread writeThread_;

	std::queue<std::string> writeQueue_;
	std::mutex writeMutex_;
	std::condition_variable writeCv_;
	bool writerDone_ = false;
	size_t writeQueueBytes_ = 0;

	std::queue<json> readQueue_;
	std::mutex readMutex_;
	std::condition_variable readCv_;
	std::condition_variable readSpaceCv_; // the reader waits here while readQueue_ is full

	std::atomic<bool> disconnected_;
	std::atomic<bool> abortWrites_{false};
	std::atomic<bool> spamming_{false};
};

#endif // BRIDGE_H
