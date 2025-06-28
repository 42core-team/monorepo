#ifndef BRIDGE_H
#define BRIDGE_H

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <sys/time.h>
#include <string>
#include <atomic>

#include "Logger.h"

#include "json.hpp"
using json = nlohmann::ordered_json;

class Bridge
{
	public:
		explicit Bridge(int socket_fd, unsigned int teamId);
		~Bridge();

		void sendMessage(const json& message);
		bool receiveMessage(json& message);
		bool isDisconnected();

		void start();

		unsigned int getTeamId() const { return team_id_; }
		void setTeamId(unsigned int teamId) { team_id_ = teamId; }

		const std::string& getTeamName() const { return team_name_; }
		void setTeamName(const std::string& teamName) { team_name_ = teamName; }

	private:
		void readLoop();
		void writeLoop();

		int socket_fd_;
		unsigned int team_id_;
		std::string team_name_;
		std::thread readThread_;
		std::thread writeThread_;

		std::queue<std::string> writeQueue_;
		std::mutex writeMutex_;
		std::condition_variable writeCv_;

		std::queue<json> readQueue_;
		std::mutex readMutex_;
		std::condition_variable readCv_;

		std::atomic<bool> disconnected_;
};

#endif // BRIDGE_H
