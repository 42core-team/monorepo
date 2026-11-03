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

std::string ReplayStream::configMessage(long long lastTick) const
{
	return staticConfigMessagePrefix_ + std::to_string(lastTick) + '}';
}

void ReplayStream::configure(const json &config, const json &misc)
{
	json message;
	message["type"] = "config";
	message["config"] = config;
	message["misc"] = misc;
	staticConfigMessagePrefix_ = message.dump();
	staticConfigMessagePrefix_.pop_back();
	staticConfigMessagePrefix_ += ",\"last_tick\":";
	server_.setInitialMessage(configMessage(-1));
}

void ReplayStream::addTick(unsigned long long tick, const json &state)
{
	json message;
	message["type"] = "tick";
	message["tick"] = tick;
	message["data"] = state;
	server_.publish(message.dump(), configMessage(static_cast<long long>(tick)));
}

void ReplayStream::finish(const json &misc, unsigned long long finalTick)
{
	json message;
	message["type"] = "end";
	message["misc"] = misc;
	message["full_tick_amount"] = finalTick;
	server_.publish(message.dump());
}
