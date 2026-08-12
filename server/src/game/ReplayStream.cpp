#include "ReplayStream.h"

ReplayStream::ReplayStream() : server_("/replay")
{
}

ReplayStream &ReplayStream::instance()
{
	static ReplayStream stream;
	return stream;
}

void ReplayStream::start(uint16_t port)
{
	server_.start(port);
}

json ReplayStream::configMessage(long long lastTick) const
{
	json message;
	message["type"] = "config";
	message["config"] = config_;
	message["misc"] = misc_;
	message["last_tick"] = lastTick;
	return message;
}

void ReplayStream::configure(const json &config, const json &misc)
{
	config_ = config;
	misc_ = misc;
	server_.setInitialMessage(configMessage(-1).dump());
}

void ReplayStream::addTick(unsigned long long tick, const json &state)
{
	json message;
	message["type"] = "tick";
	message["tick"] = tick;
	message["data"] = state;
	server_.publish(message.dump(), configMessage(static_cast<long long>(tick)).dump());
}

void ReplayStream::finish(const json &misc, unsigned long long finalTick)
{
	json message;
	message["type"] = "end";
	message["misc"] = misc;
	message["full_tick_amount"] = finalTick;
	server_.publish(message.dump());
}
