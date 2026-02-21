#ifndef PLAYER_SESSION_H
#define PLAYER_SESSION_H

#include "Action.h"
#include "core_game.grpc.pb.h"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

class PlayerSession
{
  public:
	explicit PlayerSession(unsigned int teamId, const std::string &name);
	~PlayerSession() = default;

	PlayerSession(const PlayerSession &) = delete;
	PlayerSession &operator=(const PlayerSession &) = delete;

	// ---- Tick lifecycle (called by game loop) ----

	// Write a TickSignal to the client's bidi stream
	void signalTickStart(uint64_t tick, const std::vector<std::string> &errors, bool gameOver,
						 int winnerTeamId = -1);

	// Block until client sends EndTurnRequest or timeout
	bool waitForEndTurn(unsigned int timeoutMs);

	// Called by GameServiceImpl when it reads EndTurnRequest from the stream
	void notifyEndTurn();

	// Signal game over (final TickSignal)
	void signalGameOver(uint64_t tick, const std::vector<std::string> &errors, int winnerTeamId);

	// ---- Action queue (called by gRPC action RPC threads) ----

	void addAction(std::unique_ptr<Action> action);
	std::vector<std::unique_ptr<Action>> drainActions();

	// ---- Debug data queue ----

	void addDebugData(const core_game::DebugDataEntry &entry);
	std::vector<core_game::DebugDataEntry> drainDebugData();

	// ---- Stream handle ----

	void setStream(grpc::ServerReaderWriter<core_game::TickSignal, core_game::EndTurnRequest> *stream);

	// ---- Disconnect tracking ----

	bool isDisconnected() const { return disconnected_.load(); }
	void setDisconnected() { disconnected_ = true; }

	// ---- Accessors ----

	unsigned int getTeamId() const { return teamId_; }
	const std::string &getName() const { return name_; }

  private:
	unsigned int teamId_;
	std::string name_;

	// Bidi stream handle (owned by gRPC, lifetime managed by TickStream RPC)
	grpc::ServerReaderWriter<core_game::TickSignal, core_game::EndTurnRequest> *stream_ = nullptr;
	std::mutex streamMutex_;

	// EndTurn signaling
	std::mutex endTurnMutex_;
	std::condition_variable endTurnCv_;
	bool endTurnReceived_ = false;

	// Pending actions from gRPC threads
	std::mutex actionsMutex_;
	std::vector<std::unique_ptr<Action>> pendingActions_;

	// Pending debug data
	std::mutex debugMutex_;
	std::vector<core_game::DebugDataEntry> pendingDebugData_;

	std::atomic<bool> disconnected_{false};
};

#endif // PLAYER_SESSION_H
