#include "GameServiceImpl.h"

#include "Board.h"
#include "Bomb.h"
#include "Config.h"
#include "Core.h"
#include "Deposit.h"
#include "GemPile.h"
#include "Logger.h"
#include "ReplayEncoder.h"
#include "Unit.h"
#include "json.hpp"

#include <algorithm>
using json = nlohmann::ordered_json;

// ---------------------------------------------------------------------------
// helpers
// ---------------------------------------------------------------------------

static std::string sanitizeTeamName(const std::string &teamName, const std::string &defaultName = "Unnamed")
{
	constexpr size_t MAX_LEN = 16;
	std::string sanitized;
	sanitized.reserve(std::min(teamName.size(), MAX_LEN));
	for (unsigned char uc : teamName)
	{
		if ((std::isalnum(uc) != 0) || uc == '_' || uc == '.' || uc == '-')
		{
			sanitized += static_cast<char>(uc);
			if (sanitized.size() >= MAX_LEN) break;
		}
	}
	if (sanitized.empty())
	{
		Logger::LogWarn("Team name '" + teamName + "' is invalid, using default name '" + defaultName + "'");
		return defaultName;
	}
	return sanitized;
}

// ---------------------------------------------------------------------------
// ctor
// ---------------------------------------------------------------------------

GameServiceImpl::GameServiceImpl(const std::vector<unsigned int> &expectedTeamIds)
	: expectedTeamIds_(expectedTeamIds)
{
}

// ---------------------------------------------------------------------------
// session management
// ---------------------------------------------------------------------------

bool GameServiceImpl::waitForAllLogins(unsigned int timeoutMs)
{
	std::unique_lock<std::mutex> lock(loginMutex_);
	return loginCv_.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] {
		std::lock_guard<std::mutex> sLock(sessionsMutex_);
		return sessions_.size() >= expectedTeamIds_.size();
	});
}

std::vector<std::shared_ptr<PlayerSession>> GameServiceImpl::getAllSessions()
{
	std::lock_guard<std::mutex> lock(sessionsMutex_);
	std::vector<std::shared_ptr<PlayerSession>> out;
	out.reserve(sessions_.size());
	for (auto &pair : sessions_)
		out.push_back(pair.second);
	return out;
}

std::shared_ptr<PlayerSession> GameServiceImpl::getSession(unsigned int teamId)
{
	std::lock_guard<std::mutex> lock(sessionsMutex_);
	auto it = sessions_.find(teamId);
	if (it == sessions_.end()) return nullptr;
	return it->second;
}

std::vector<unsigned int> GameServiceImpl::getConnectedTeamIds()
{
	std::lock_guard<std::mutex> lock(sessionsMutex_);
	std::vector<unsigned int> ids;
	for (auto &pair : sessions_)
	{
		if (!pair.second->isDisconnected()) ids.push_back(pair.first);
	}
	return ids;
}

const core_game::GameConfigProto &GameServiceImpl::getConfigProto()
{
	std::lock_guard<std::mutex> lock(configMutex_);
	if (configBuilt_) return configProto_;

	const auto &gc = Config::game();
	configProto_.set_grid_size(gc.gridSize);
	configProto_.set_idle_income(gc.idleIncome);
	configProto_.set_idle_income_timeout(gc.idleIncomeTimeOut);
	configProto_.set_deposit_hp(gc.depositHp);
	configProto_.set_deposit_income(gc.depositIncome);
	configProto_.set_gem_pile_income(gc.gemPileIncome);
	configProto_.set_core_hp(gc.coreHp);
	configProto_.set_core_spawn_cooldown(gc.coreSpawnCooldown);
	configProto_.set_initial_balance(gc.initialBalance);
	configProto_.set_wall_hp(gc.wallHp);
	configProto_.set_wall_build_cost(gc.wallBuildCost);
	configProto_.set_bomb_hp(gc.bombHp);
	configProto_.set_bomb_countdown(gc.bombCountdown);
	configProto_.set_bomb_throw_cost(gc.bombThrowCost);
	configProto_.set_bomb_reach(gc.bombReach);
	configProto_.set_bomb_damage_core(gc.bombDamageCore);
	configProto_.set_bomb_damage_unit(gc.bombDamageUnit);
	configProto_.set_bomb_damage_deposit(gc.bombDamageDeposit);

	for (const auto &uc : gc.units)
	{
		auto *up = configProto_.add_units();
		up->set_name(uc.name);
		up->set_cost(uc.cost);
		up->set_hp(uc.hp);
		up->set_base_action_cooldown(uc.baseActionCooldown);
		up->set_max_action_cooldown(uc.maxActionCooldown);
		up->set_balance_per_cooldown_step(uc.balancePerCooldownStep);
		up->set_damage_core(uc.damageCore);
		up->set_damage_unit(uc.damageUnit);
		up->set_damage_deposit(uc.damageDeposit);
		up->set_damage_wall(uc.damageWall);
		up->set_damage_bomb(uc.damageBomb);
		switch (uc.buildType)
		{
		case BuildType::WALL:
			up->set_build_type("wall");
			break;
		case BuildType::BOMB:
			up->set_build_type("bomb");
			break;
		default:
			up->set_build_type("none");
			break;
		}
	}

	for (const auto &cp : gc.corePositions)
	{
		auto *pos = configProto_.add_core_positions();
		pos->set_x(cp.x);
		pos->set_y(cp.y);
	}

	configProto_.set_raw_json(Config::encodeConfig().dump());

	configBuilt_ = true;
	return configProto_;
}

// ---------------------------------------------------------------------------
// metadata helper
// ---------------------------------------------------------------------------

unsigned int GameServiceImpl::extractTeamId(grpc::ServerContext *context)
{
	auto meta = context->client_metadata();
	auto it = meta.find("x-team-id");
	if (it != meta.end())
	{
		return static_cast<unsigned int>(std::stoul(std::string(it->second.data(), it->second.size())));
	}
	return 0;
}

// ---------------------------------------------------------------------------
// fillGameObject — mirrors StateEncoder::encodeFullState()
// ---------------------------------------------------------------------------

void GameServiceImpl::fillGameObject(core_game::GameObject *proto, const Object &obj)
{
	proto->set_id(obj.getId());
	proto->set_type(static_cast<int32_t>(obj.getType()));

	Position pos = Board::instance().getObjectPositionById(obj.getId());
	auto *protoPos = proto->mutable_pos();
	protoPos->set_x(pos.x);
	protoPos->set_y(pos.y);

	proto->set_hp(obj.getHP());

	if (obj.getType() == ObjectType::Core)
	{
		const auto &core = static_cast<const Core &>(obj);
		proto->set_team_id(core.getTeamId());
		proto->set_gems(core.getBalance());
		proto->set_spawn_cooldown(core.getSpawnCooldown());
	}
	else if (obj.getType() == ObjectType::Unit)
	{
		const auto &unit = static_cast<const Unit &>(obj);
		proto->set_team_id(unit.getTeamId());
		proto->set_unit_type(unit.getUnitType());
		proto->set_gems(unit.getBalance());
		proto->set_action_cooldown(unit.getActionCooldown());
	}
	else if (obj.getType() == ObjectType::Deposit)
	{
		const auto &dep = static_cast<const Deposit &>(obj);
		proto->set_gems(dep.getBalance());
	}
	else if (obj.getType() == ObjectType::GemPile)
	{
		const auto &gp = static_cast<const GemPile &>(obj);
		proto->set_gems(gp.getBalance());
	}
	else if (obj.getType() == ObjectType::Bomb)
	{
		const auto &bomb = static_cast<const Bomb &>(obj);
		proto->set_countdown(bomb.getCountdown());
		proto->set_countdown_started(bomb.isCountdownStarted());
		for (const Position &p : bomb.explosionTiles_)
		{
			auto *tile = proto->add_explosion_tiles();
			tile->set_x(p.x);
			tile->set_y(p.y);
		}
	}
}

// ---------------------------------------------------------------------------
// Login
// ---------------------------------------------------------------------------

grpc::Status GameServiceImpl::Login(grpc::ServerContext * /*context*/, const core_game::LoginRequest *request,
									core_game::LoginResponse *response)
{
	unsigned int teamId = request->team_id();
	const std::string &password = request->password();
	const std::string &rawName = request->name();

	if (password != "42")
	{
		response->set_success(false);
		response->set_error("Invalid password");
		return grpc::Status::OK;
	}

	if (std::find(expectedTeamIds_.begin(), expectedTeamIds_.end(), teamId) == expectedTeamIds_.end())
	{
		response->set_success(false);
		response->set_error("Unexpected team ID " + std::to_string(teamId));
		return grpc::Status::OK;
	}

	{
		std::lock_guard<std::mutex> lock(sessionsMutex_);
		if (sessions_.find(teamId) != sessions_.end())
		{
			response->set_success(false);
			response->set_error("Duplicate login for team ID " + std::to_string(teamId));
			return grpc::Status::OK;
		}
	}

	std::string teamName = sanitizeTeamName(rawName, "Team" + std::to_string(teamId));

	// Update replay encoder name (allow client to override only if current name is the fallback)
	const std::string current = ReplayEncoder::instance().getTeamNameFromTeamId(teamId);
	const std::string fallback = "Team" + std::to_string(teamId);
	if (current == fallback)
	{
		ReplayEncoder::instance().setTeamName(teamId, teamName);
	}
	ReplayEncoder::instance().markConnectedInitially(teamId, true);

	auto session = std::make_shared<PlayerSession>(teamId, teamName);

	{
		std::lock_guard<std::mutex> lock(sessionsMutex_);
		sessions_[teamId] = session;
	}
	loginCv_.notify_all();

	Logger::Log("Team " + std::to_string(teamId) + " (" + teamName + ") logged in via gRPC.");

	response->set_success(true);
	*response->mutable_config() = getConfigProto();
	return grpc::Status::OK;
}

// ---------------------------------------------------------------------------
// TickStream (bidirectional)
// ---------------------------------------------------------------------------

grpc::Status GameServiceImpl::TickStream(
		grpc::ServerContext *context,
		grpc::ServerReaderWriter<core_game::TickSignal, core_game::EndTurnRequest> *stream)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Not logged in or invalid team ID");
	}

	session->setStream(stream);

	// Read EndTurnRequest messages from the client until stream ends or client disconnects
	core_game::EndTurnRequest req;
	while (stream->Read(&req))
	{
		session->notifyEndTurn();
	}

	// Stream ended — client disconnected
	session->setDisconnected();
	session->notifyEndTurn(); // wake up any waiting game loop
	Logger::LogWarn("TickStream ended for team " + std::to_string(teamId));

	return grpc::Status::OK;
}

// ---------------------------------------------------------------------------
// Action RPCs
// ---------------------------------------------------------------------------

grpc::Status GameServiceImpl::CreateUnit(grpc::ServerContext *context, const core_game::CreateUnitRequest *request,
										 core_game::ActionResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		response->set_success(false);
		response->set_error("Not logged in");
		return grpc::Status::OK;
	}

	json msg;
	msg["type"] = "create";
	msg["unit_type"] = request->unit_type();
	session->addAction(std::make_unique<CreateAction>(msg));

	response->set_success(true);
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::Move(grpc::ServerContext *context, const core_game::MoveRequest *request,
								   core_game::ActionResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		response->set_success(false);
		response->set_error("Not logged in");
		return grpc::Status::OK;
	}

	json msg;
	msg["type"] = "move";
	msg["unit_id"] = request->unit_id();
	msg["x"] = request->target().x();
	msg["y"] = request->target().y();
	session->addAction(std::make_unique<MoveAction>(msg));

	response->set_success(true);
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::Attack(grpc::ServerContext *context, const core_game::AttackRequest *request,
									 core_game::ActionResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		response->set_success(false);
		response->set_error("Not logged in");
		return grpc::Status::OK;
	}

	json msg;
	msg["type"] = "attack";
	msg["unit_id"] = request->unit_id();
	msg["target_id"] = request->target_id();
	session->addAction(std::make_unique<AttackAction>(msg));

	response->set_success(true);
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::TransferGems(grpc::ServerContext *context, const core_game::TransferGemsRequest *request,
										   core_game::ActionResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		response->set_success(false);
		response->set_error("Not logged in");
		return grpc::Status::OK;
	}

	json msg;
	msg["type"] = "transfer_gems";
	msg["source_id"] = request->source_id();
	msg["x"] = request->target().x();
	msg["y"] = request->target().y();
	msg["amount"] = request->amount();
	session->addAction(std::make_unique<TransferGemsAction>(msg));

	response->set_success(true);
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::Build(grpc::ServerContext *context, const core_game::BuildRequest *request,
									core_game::ActionResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		response->set_success(false);
		response->set_error("Not logged in");
		return grpc::Status::OK;
	}

	json msg;
	msg["type"] = "build";
	msg["unit_id"] = request->unit_id();
	msg["x"] = request->target().x();
	msg["y"] = request->target().y();
	session->addAction(std::make_unique<BuildAction>(msg));

	response->set_success(true);
	return grpc::Status::OK;
}

// ---------------------------------------------------------------------------
// Debug data
// ---------------------------------------------------------------------------

grpc::Status GameServiceImpl::SendDebugData(grpc::ServerContext *context, const core_game::DebugDataRequest *request,
											core_game::DebugDataResponse * /*response*/)
{
	unsigned int teamId = extractTeamId(context);
	auto session = getSession(teamId);
	if (!session)
	{
		return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Not logged in");
	}

	for (const auto &entry : request->entries())
	{
		session->addDebugData(entry);
	}
	return grpc::Status::OK;
}

// ---------------------------------------------------------------------------
// Query RPCs
// ---------------------------------------------------------------------------

grpc::Status GameServiceImpl::GetObjectById(grpc::ServerContext * /*context*/,
											const core_game::GetObjectByIdRequest *request,
											core_game::GetObjectByIdResponse *response)
{
	Object *obj = Board::instance().getObjectById(request->id());
	if (obj)
	{
		fillGameObject(response->mutable_object(), *obj);
	}
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetObjectAtPos(grpc::ServerContext * /*context*/,
											 const core_game::GetObjectAtPosRequest *request,
											 core_game::GetObjectAtPosResponse *response)
{
	Position pos(request->pos().x(), request->pos().y());
	Object *obj = Board::instance().getObjectAtPos(pos);
	if (obj)
	{
		fillGameObject(response->mutable_object(), *obj);
	}
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetMyCore(grpc::ServerContext *context,
										const core_game::GetMyCoreRequest * /*request*/,
										core_game::GetMyCoreResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	Core *core = Board::instance().getCoreByTeamId(teamId);
	if (core)
	{
		fillGameObject(response->mutable_core(), *core);
	}
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetMyUnits(grpc::ServerContext *context,
										 const core_game::GetMyUnitsRequest * /*request*/,
										 core_game::GetMyUnitsResponse *response)
{
	unsigned int teamId = extractTeamId(context);
	for (auto &obj : Board::instance())
	{
		if (obj.getType() == ObjectType::Unit)
		{
			const auto &unit = static_cast<const Unit &>(obj);
			if (unit.getTeamId() == teamId)
			{
				fillGameObject(response->add_units(), obj);
			}
		}
	}
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetConfig(grpc::ServerContext * /*context*/,
										const core_game::GetConfigRequest * /*request*/,
										core_game::GetConfigResponse *response)
{
	*response->mutable_config() = getConfigProto();
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetTick(grpc::ServerContext * /*context*/,
									  const core_game::GetTickRequest * /*request*/,
									  core_game::GetTickResponse *response)
{
	response->set_tick(currentTick_.load());
	return grpc::Status::OK;
}

grpc::Status GameServiceImpl::GetAllObjects(grpc::ServerContext * /*context*/,
											const core_game::GetAllObjectsRequest * /*request*/,
											core_game::GetAllObjectsResponse *response)
{
	for (auto &obj : Board::instance())
	{
		fillGameObject(response->add_objects(), obj);
	}
	return grpc::Status::OK;
}
