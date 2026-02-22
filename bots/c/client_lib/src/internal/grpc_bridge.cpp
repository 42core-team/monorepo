#include "grpc_bridge.h"

#include "core_lib_internal.h"

#include <chrono>
#include <core_game.grpc.pb.h>
#include <core_game.pb.h>
#include <cstdlib>
#include <cstring>
#include <grpcpp/grpcpp.h>
#include <memory>
#include <string>
#include <unistd.h>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Globals
// ─────────────────────────────────────────────────────────────────────────────

static std::shared_ptr<grpc::Channel> g_channel;
static std::unique_ptr<core_game::CoreGameService::Stub> g_stub;
static unsigned long g_team_id = 0;

// Bidirectional tick stream
static std::unique_ptr<grpc::ClientContext> g_tick_ctx;
static std::unique_ptr<grpc::ClientReaderWriter<core_game::EndTurnRequest, core_game::TickSignal>> g_tick_stream;

// ─────────────────────────────────────────────────────────────────────────────
// Per-tick object cache
// ─────────────────────────────────────────────────────────────────────────────

static t_obj **g_cache = nullptr;
static int g_cache_count = 0;
static int g_cache_capacity = 0;

static t_obj *cache_store(const t_obj &obj)
{
	if (g_cache == nullptr)
	{
		g_cache_capacity = 64;
		g_cache = (t_obj **)malloc(sizeof(t_obj *) * g_cache_capacity);
	}
	else if (g_cache_count >= g_cache_capacity)
	{
		g_cache_capacity *= 2;
		g_cache = (t_obj **)realloc(g_cache, sizeof(t_obj *) * g_cache_capacity);
	}

	t_obj *copy = (t_obj *)malloc(sizeof(t_obj));
	*copy = obj;
	g_cache[g_cache_count++] = copy;
	return copy;
}

extern "C" void grpc_bridge_cache_clear(void)
{
	for (int i = 0; i < g_cache_count; i++)
		free(g_cache[i]);
	free(g_cache);
	g_cache = nullptr;
	g_cache_count = 0;
	g_cache_capacity = 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Protobuf → C struct conversion
// ─────────────────────────────────────────────────────────────────────────────

static t_obj proto_to_obj(const core_game::GameObject &proto)
{
	t_obj obj;
	memset(&obj, 0, sizeof(obj));

	obj.id = proto.id();
	obj.type = (t_obj_type)proto.type();
	obj.pos.x = (unsigned short)proto.pos().x();
	obj.pos.y = (unsigned short)proto.pos().y();
	obj.hp = (unsigned long)proto.hp();

	switch (obj.type)
	{
	case OBJ_CORE:
		if (proto.has_team_id()) obj.s_core.team_id = proto.team_id();
		if (proto.has_gems()) obj.s_core.gems = proto.gems();
		if (proto.has_spawn_cooldown()) obj.s_core.spawn_cooldown = proto.spawn_cooldown();
		break;
	case OBJ_UNIT:
		if (proto.has_unit_type()) obj.s_unit.unit_type = (t_unit_type)proto.unit_type();
		if (proto.has_team_id()) obj.s_unit.team_id = proto.team_id();
		if (proto.has_gems()) obj.s_unit.gems = proto.gems();
		if (proto.has_action_cooldown()) obj.s_unit.action_cooldown = proto.action_cooldown();
		break;
	case OBJ_DEPOSIT:
	case OBJ_GEM_PILE:
		if (proto.has_gems()) obj.s_deposit_gems_pile.gems = proto.gems();
		break;
	case OBJ_BOMB:
		if (proto.has_countdown()) obj.s_bomb.countdown = proto.countdown();
		break;
	default:
		break;
	}

	return obj;
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper: create a ClientContext with team_id metadata
// ─────────────────────────────────────────────────────────────────────────────

static std::unique_ptr<grpc::ClientContext> make_ctx()
{
	auto ctx = std::make_unique<grpc::ClientContext>();
	ctx->AddMetadata("x-team-id", std::to_string(g_team_id));
	return ctx;
}

// ─────────────────────────────────────────────────────────────────────────────
// Connection & Auth
// ─────────────────────────────────────────────────────────────────────────────

static void parse_config(const core_game::GameConfigProto &cfg)
{
	game.config.gridSize = cfg.grid_size();
	game.config.idle_income = cfg.idle_income();
	game.config.idle_income_timeout = cfg.idle_income_timeout();
	game.config.deposit_hp = cfg.deposit_hp();
	game.config.deposit_income = cfg.deposit_income();
	game.config.gem_pile_income = cfg.gem_pile_income();
	game.config.core_hp = cfg.core_hp();
	game.config.core_spawn_cooldown = cfg.core_spawn_cooldown();
	game.config.initial_balance = cfg.initial_balance();
	game.config.wall_hp = cfg.wall_hp();
	game.config.wall_build_cost = cfg.wall_build_cost();
	game.config.bomb_countdown = cfg.bomb_countdown();
	game.config.bomb_throw_cost = cfg.bomb_throw_cost();
	game.config.bomb_reach = cfg.bomb_reach();
	game.config.bomb_damage_core = cfg.bomb_damage_core();
	game.config.bomb_damage_unit = cfg.bomb_damage_unit();
	game.config.bomb_damage_deposit = cfg.bomb_damage_deposit();

	int unit_count = cfg.units_size();
	game.config.units = (t_unit_config **)malloc(sizeof(t_unit_config *) * (unit_count + 1));
	for (int i = 0; i < unit_count; i++)
	{
		const auto &u = cfg.units(i);
		t_unit_config *uc = (t_unit_config *)malloc(sizeof(t_unit_config));
		uc->name = strdup(u.name().c_str());
		uc->unit_type = (t_unit_type)i;
		uc->cost = u.cost();
		uc->hp = u.hp();
		uc->baseActionCooldown = u.base_action_cooldown();
		uc->maxActionCooldown = u.max_action_cooldown();
		uc->balancePerCooldownStep = u.balance_per_cooldown_step();
		uc->dmg_core = u.damage_core();
		uc->dmg_unit = u.damage_unit();
		uc->dmg_deposit = u.damage_deposit();
		uc->dmg_wall = u.damage_wall();
		uc->dmg_bomb = u.damage_bomb();
		const std::string &bt = u.build_type();
		if (bt == "wall")
			uc->build_type = BUILD_TYPE_WALL;
		else if (bt == "bomb")
			uc->build_type = BUILD_TYPE_BOMB;
		else
			uc->build_type = BUILD_TYPE_NONE;
		game.config.units[i] = uc;
	}
	game.config.units[unit_count] = nullptr;
}

extern "C" int grpc_bridge_connect(const char *host, int port)
{
	std::string target = std::string(host) + ":" + std::to_string(port);
	g_channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	g_stub = core_game::CoreGameService::NewStub(g_channel);

	// Wait for the channel to be ready, retrying like the old TCP socket loop
	for (;;)
	{
		auto state = g_channel->GetState(true); // true = try to connect
		if (state == GRPC_CHANNEL_READY) break;


		// Wait up to 2 seconds for a state change, then retry
		auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(2);
		g_channel->WaitForStateChange(state, deadline);

		state = g_channel->GetState(false);
		if (state == GRPC_CHANNEL_READY) break;
		if (state == GRPC_CHANNEL_SHUTDOWN)
		{
			fprintf(stderr, "\nChannel shut down\n");
			return -1;
		}
	}

	return 0;
}

extern "C" int grpc_bridge_login(unsigned long team_id, const char *password, const char *name)
{
	g_team_id = team_id;

	core_game::LoginRequest req;
	req.set_team_id((uint32_t)team_id);
	req.set_password(password);
	req.set_name(name);

	core_game::LoginResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->Login(ctx.get(), req, &resp);

	if (!status.ok())
	{
		fprintf(stderr, "Login RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	if (!resp.success())
	{
		fprintf(stderr, "Login rejected: %s\n", resp.error().c_str());
		return -1;
	}

	parse_config(resp.config());
	game.my_team_id = team_id;

	return 0;
}

extern "C" void grpc_bridge_shutdown(void)
{
	if (g_tick_stream)
	{
		g_tick_stream->WritesDone();
		g_tick_stream->Finish();
		g_tick_stream.reset();
	}
	g_tick_ctx.reset();
	g_stub.reset();
	g_channel.reset();
	grpc_bridge_cache_clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick lifecycle
// ─────────────────────────────────────────────────────────────────────────────

extern "C" int grpc_bridge_start_tick_stream(void)
{
	g_tick_ctx = std::make_unique<grpc::ClientContext>();
	g_tick_ctx->AddMetadata("x-team-id", std::to_string(g_team_id));
	g_tick_stream = g_stub->TickStream(g_tick_ctx.get());
	if (!g_tick_stream)
	{
		fprintf(stderr, "Failed to open TickStream\n");
		return -1;
	}
	return 0;
}

extern "C" int grpc_bridge_wait_tick(unsigned long *out_tick, char ***out_errors, int *out_error_count,
									 bool *out_game_over, unsigned long *out_winner_team_id)
{
	core_game::TickSignal signal;
	if (!g_tick_stream->Read(&signal))
	{
		// Stream ended
		*out_game_over = true;
		return -1;
	}

	*out_tick = (unsigned long)signal.tick();
	*out_game_over = signal.game_over();
	if (signal.has_winner_team_id())
		*out_winner_team_id = signal.winner_team_id();
	else
		*out_winner_team_id = 0;

	int err_count = signal.errors_size();
	*out_error_count = err_count;
	if (err_count > 0)
	{
		char **errs = (char **)malloc(sizeof(char *) * (err_count + 1));
		for (int i = 0; i < err_count; i++)
			errs[i] = strdup(signal.errors(i).c_str());
		errs[err_count] = nullptr;
		*out_errors = errs;
	}
	else
	{
		*out_errors = nullptr;
	}

	return 0;
}

extern "C" int grpc_bridge_end_turn(void)
{
	core_game::EndTurnRequest req;
	if (!g_tick_stream->Write(req))
	{
		fprintf(stderr, "Failed to write EndTurnRequest\n");
		return -1;
	}
	return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Actions
// ─────────────────────────────────────────────────────────────────────────────

extern "C" int grpc_bridge_create_unit(unsigned long unit_type)
{
	core_game::CreateUnitRequest req;
	req.set_unit_type((uint32_t)unit_type);

	core_game::ActionResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->CreateUnit(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "CreateUnit RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

extern "C" int grpc_bridge_move(unsigned long unit_id, unsigned short x, unsigned short y)
{
	core_game::MoveRequest req;
	req.set_unit_id((uint32_t)unit_id);
	req.mutable_target()->set_x(x);
	req.mutable_target()->set_y(y);

	core_game::ActionResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->Move(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "Move RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

extern "C" int grpc_bridge_attack(unsigned long unit_id, unsigned long target_id)
{
	core_game::AttackRequest req;
	req.set_unit_id((uint32_t)unit_id);
	req.set_target_id((uint32_t)target_id);

	core_game::ActionResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->Attack(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "Attack RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

extern "C" int grpc_bridge_transfer_gems(unsigned long source_id, unsigned short x, unsigned short y,
										 unsigned long amount)
{
	core_game::TransferGemsRequest req;
	req.set_source_id((uint32_t)source_id);
	req.mutable_target()->set_x(x);
	req.mutable_target()->set_y(y);
	req.set_amount((uint32_t)amount);

	core_game::ActionResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->TransferGems(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "TransferGems RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

extern "C" int grpc_bridge_build(unsigned long unit_id, unsigned short x, unsigned short y)
{
	core_game::BuildRequest req;
	req.set_unit_id((uint32_t)unit_id);
	req.mutable_target()->set_x(x);
	req.mutable_target()->set_y(y);

	core_game::ActionResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->Build(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "Build RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Debug data
// ─────────────────────────────────────────────────────────────────────────────

extern "C" int grpc_bridge_send_debug_data(void)
{
	if (debug_data.count == 0) return 0;

	core_game::DebugDataRequest req;
	for (unsigned int i = 0; i < debug_data.count; i++)
	{
		t_debug_entry *e = &debug_data.entries[i];
		auto *entry = req.add_entries();
		entry->set_object_id((uint32_t)e->object_id);
		if (e->info) entry->set_object_info(e->info);

		t_debug_path_node *node = e->path;
		while (node)
		{
			auto *p = entry->add_object_path();
			p->set_x(node->pos.x);
			p->set_y(node->pos.y);
			node = node->next;
		}
	}

	core_game::DebugDataResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->SendDebugData(ctx.get(), req, &resp);
	if (!status.ok())
	{
		fprintf(stderr, "SendDebugData RPC failed: %s\n", status.error_message().c_str());
		return -1;
	}
	return 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// Queries
// ─────────────────────────────────────────────────────────────────────────────

extern "C" t_obj *grpc_bridge_get_object_by_id(unsigned long id)
{
	core_game::GetObjectByIdRequest req;
	req.set_id((uint32_t)id);

	core_game::GetObjectByIdResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetObjectById(ctx.get(), req, &resp);
	if (!status.ok() || !resp.has_object()) return nullptr;

	t_obj obj = proto_to_obj(resp.object());
	return cache_store(obj);
}

extern "C" t_obj *grpc_bridge_get_object_at_pos(unsigned short x, unsigned short y)
{
	core_game::GetObjectAtPosRequest req;
	req.mutable_pos()->set_x(x);
	req.mutable_pos()->set_y(y);

	core_game::GetObjectAtPosResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetObjectAtPos(ctx.get(), req, &resp);
	if (!status.ok() || !resp.has_object()) return nullptr;

	t_obj obj = proto_to_obj(resp.object());
	return cache_store(obj);
}

extern "C" t_obj *grpc_bridge_get_my_core(void)
{
	core_game::GetMyCoreRequest req;
	core_game::GetMyCoreResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetMyCore(ctx.get(), req, &resp);
	if (!status.ok() || !resp.has_core()) return nullptr;

	t_obj obj = proto_to_obj(resp.core());
	return cache_store(obj);
}

extern "C" t_obj **grpc_bridge_get_my_units(void)
{
	core_game::GetMyUnitsRequest req;
	core_game::GetMyUnitsResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetMyUnits(ctx.get(), req, &resp);
	if (!status.ok()) return nullptr;

	int count = resp.units_size();
	if (count == 0) return nullptr;

	t_obj **result = (t_obj **)malloc(sizeof(t_obj *) * (count + 1));
	for (int i = 0; i < count; i++)
	{
		t_obj obj = proto_to_obj(resp.units(i));
		result[i] = cache_store(obj);
	}
	result[count] = nullptr;
	return result;
}

extern "C" t_obj **grpc_bridge_get_all_objects(void)
{
	core_game::GetAllObjectsRequest req;
	core_game::GetAllObjectsResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetAllObjects(ctx.get(), req, &resp);
	if (!status.ok()) return nullptr;

	int count = resp.objects_size();
	if (count == 0)
	{
		t_obj **result = (t_obj **)malloc(sizeof(t_obj *));
		result[0] = nullptr;
		return result;
	}

	t_obj **result = (t_obj **)malloc(sizeof(t_obj *) * (count + 1));
	for (int i = 0; i < count; i++)
	{
		t_obj obj = proto_to_obj(resp.objects(i));
		result[i] = cache_store(obj);
	}
	result[count] = nullptr;
	return result;
}

extern "C" unsigned long grpc_bridge_get_tick(void)
{
	core_game::GetTickRequest req;
	core_game::GetTickResponse resp;
	auto ctx = make_ctx();
	grpc::Status status = g_stub->GetTick(ctx.get(), req, &resp);
	if (!status.ok()) return 0;
	return (unsigned long)resp.tick();
}
