#ifndef GAME_SERVICE_IMPL_H
#define GAME_SERVICE_IMPL_H

#include "PlayerSession.h"
#include "core_game.grpc.pb.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class GameServiceImpl final : public core_game::CoreGameService::Service
{
  public:
	explicit GameServiceImpl(const std::vector<unsigned int> &expectedTeamIds);

	// ---- Session management (called from main / game loop) ----

	// Blocks until all expected teams have logged in, or timeout expires.
	// Returns true if all connected, false if timeout.
	bool waitForAllLogins(unsigned int timeoutMs);

	// Returns sessions for connected teams (call after waitForAllLogins)
	std::vector<std::shared_ptr<PlayerSession>> getAllSessions();
	std::shared_ptr<PlayerSession> getSession(unsigned int teamId);

	// Get the set of team IDs that successfully connected
	std::vector<unsigned int> getConnectedTeamIds();

	// Current tick number (updated by game loop)
	void setCurrentTick(uint64_t tick) { currentTick_ = tick; }
	uint64_t getCurrentTick() const { return currentTick_; }

	// Build the GameConfigProto from server config (cached after first call)
	const core_game::GameConfigProto &getConfigProto();

	// ---- gRPC service methods ----

	grpc::Status Login(grpc::ServerContext *context, const core_game::LoginRequest *request,
					   core_game::LoginResponse *response) override;

	grpc::Status TickStream(grpc::ServerContext *context,
							grpc::ServerReaderWriter<core_game::TickSignal, core_game::EndTurnRequest> *stream) override;

	grpc::Status CreateUnit(grpc::ServerContext *context, const core_game::CreateUnitRequest *request,
							core_game::ActionResponse *response) override;

	grpc::Status Move(grpc::ServerContext *context, const core_game::MoveRequest *request,
					  core_game::ActionResponse *response) override;

	grpc::Status Attack(grpc::ServerContext *context, const core_game::AttackRequest *request,
						core_game::ActionResponse *response) override;

	grpc::Status TransferGems(grpc::ServerContext *context, const core_game::TransferGemsRequest *request,
							  core_game::ActionResponse *response) override;

	grpc::Status Build(grpc::ServerContext *context, const core_game::BuildRequest *request,
					   core_game::ActionResponse *response) override;

	grpc::Status SendDebugData(grpc::ServerContext *context, const core_game::DebugDataRequest *request,
							   core_game::DebugDataResponse *response) override;

	grpc::Status GetObjectById(grpc::ServerContext *context, const core_game::GetObjectByIdRequest *request,
							   core_game::GetObjectByIdResponse *response) override;

	grpc::Status GetObjectAtPos(grpc::ServerContext *context, const core_game::GetObjectAtPosRequest *request,
								core_game::GetObjectAtPosResponse *response) override;

	grpc::Status GetMyCore(grpc::ServerContext *context, const core_game::GetMyCoreRequest *request,
						   core_game::GetMyCoreResponse *response) override;

	grpc::Status GetMyUnits(grpc::ServerContext *context, const core_game::GetMyUnitsRequest *request,
							core_game::GetMyUnitsResponse *response) override;

	grpc::Status GetConfig(grpc::ServerContext *context, const core_game::GetConfigRequest *request,
						   core_game::GetConfigResponse *response) override;

	grpc::Status GetTick(grpc::ServerContext *context, const core_game::GetTickRequest *request,
						 core_game::GetTickResponse *response) override;

	grpc::Status GetAllObjects(grpc::ServerContext *context, const core_game::GetAllObjectsRequest *request,
							   core_game::GetAllObjectsResponse *response) override;

  private:
	// Extract team_id from gRPC metadata
	unsigned int extractTeamId(grpc::ServerContext *context);

	// Fill a GameObject proto from a board Object
	static void fillGameObject(core_game::GameObject *proto, const Object &obj);

	std::vector<unsigned int> expectedTeamIds_;

	std::mutex sessionsMutex_;
	std::unordered_map<unsigned int, std::shared_ptr<PlayerSession>> sessions_;

	// Login signaling
	std::mutex loginMutex_;
	std::condition_variable loginCv_;

	std::atomic<uint64_t> currentTick_{0};

	// Cached config proto
	std::mutex configMutex_;
	bool configBuilt_ = false;
	core_game::GameConfigProto configProto_;
};

#endif // GAME_SERVICE_IMPL_H
