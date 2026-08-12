#ifndef REPLAY_STREAM_H
#define REPLAY_STREAM_H

#include "WebSocketServer.h"
#include "json.hpp"

#include <cstdint>

using json = nlohmann::ordered_json;

class ReplayStream
{
  public:
	ReplayStream();
	~ReplayStream() = default;

	ReplayStream(const ReplayStream &) = delete;
	ReplayStream &operator=(const ReplayStream &) = delete;

	static ReplayStream &instance();

	void start(uint16_t port);
	void configure(const json &config, const json &misc);
	void addTick(unsigned long long tick, const json &state);
	void finish(const json &misc, unsigned long long finalTick);

  private:
	json configMessage(long long lastTick) const;

	WebSocketServer server_;
	json config_;
	json misc_;
};

#endif // REPLAY_STREAM_H
