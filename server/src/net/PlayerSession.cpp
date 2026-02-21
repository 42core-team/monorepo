#include "PlayerSession.h"

#include "Logger.h"

PlayerSession::PlayerSession(unsigned int teamId, const std::string &name) : teamId_(teamId), name_(name) {}

void PlayerSession::signalTickStart(uint64_t tick, const std::vector<std::string> &errors, bool gameOver,
									int winnerTeamId)
{
	// Reset endTurn flag before writing so the client can start fresh
	{
		std::lock_guard<std::mutex> etLock(endTurnMutex_);
		endTurnReceived_ = false;
	}

	std::lock_guard<std::mutex> lock(streamMutex_);
	if (!stream_ || disconnected_) return;

	core_game::TickSignal signal;
	signal.set_tick(tick);
	for (const auto &err : errors)
		signal.add_errors(err);
	signal.set_game_over(gameOver);
	if (winnerTeamId >= 0) signal.set_winner_team_id(static_cast<uint32_t>(winnerTeamId));

	if (!stream_->Write(signal))
	{
		Logger::LogWarn("Failed to write TickSignal to team " + std::to_string(teamId_) + ". Marking disconnected.");
		disconnected_ = true;
	}
}

void PlayerSession::signalGameOver(uint64_t tick, const std::vector<std::string> &errors, int winnerTeamId)
{
	signalTickStart(tick, errors, true, winnerTeamId);
}

bool PlayerSession::waitForEndTurn(unsigned int timeoutMs)
{
	std::unique_lock<std::mutex> lock(endTurnMutex_);
	if (endTurnReceived_) return true;
	if (disconnected_) return false;

	bool got = endTurnCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] {
		return endTurnReceived_ || disconnected_.load();
	});
	return got && endTurnReceived_;
}

void PlayerSession::notifyEndTurn()
{
	{
		std::lock_guard<std::mutex> lock(endTurnMutex_);
		endTurnReceived_ = true;
	}
	endTurnCv_.notify_one();
}

void PlayerSession::addAction(std::unique_ptr<Action> action)
{
	std::lock_guard<std::mutex> lock(actionsMutex_);
	pendingActions_.push_back(std::move(action));
}

std::vector<std::unique_ptr<Action>> PlayerSession::drainActions()
{
	std::lock_guard<std::mutex> lock(actionsMutex_);
	std::vector<std::unique_ptr<Action>> out;
	out.swap(pendingActions_);
	return out;
}

void PlayerSession::addDebugData(const core_game::DebugDataEntry &entry)
{
	std::lock_guard<std::mutex> lock(debugMutex_);
	pendingDebugData_.push_back(entry);
}

std::vector<core_game::DebugDataEntry> PlayerSession::drainDebugData()
{
	std::lock_guard<std::mutex> lock(debugMutex_);
	std::vector<core_game::DebugDataEntry> out;
	out.swap(pendingDebugData_);
	return out;
}

void PlayerSession::setStream(
		grpc::ServerReaderWriter<core_game::TickSignal, core_game::EndTurnRequest> *stream)
{
	std::lock_guard<std::mutex> lock(streamMutex_);
	stream_ = stream;
}
