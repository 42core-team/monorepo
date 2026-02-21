#ifndef GRPC_BRIDGE_H
#define GRPC_BRIDGE_H

#include "core_lib.h"

#ifdef __cplusplus
extern "C"
{
#endif

	// ── Connection & Auth ────────────────────────────────────────────────

	int grpc_bridge_connect(const char *host, int port);
	int grpc_bridge_login(unsigned long team_id, const char *password, const char *name);
	void grpc_bridge_shutdown(void);

	// ── Tick lifecycle (bidi stream) ─────────────────────────────────────

	/// Start the bidirectional TickStream. Must be called after login.
	int grpc_bridge_start_tick_stream(void);

	/// Block until the server sends the next TickSignal.
	/// Returns 0 on success, -1 on stream end / game over.
	/// Fills out_tick, out_game_over, out_winner_team_id.
	/// out_errors is a malloc'd NULL-terminated array of strdup'd strings (caller frees).
	int grpc_bridge_wait_tick(unsigned long *out_tick, char ***out_errors, int *out_error_count, bool *out_game_over,
							  unsigned long *out_winner_team_id);

	/// Signal that the client is done submitting actions for this tick.
	int grpc_bridge_end_turn(void);

	// ── Actions (unary RPCs) ─────────────────────────────────────────────

	int grpc_bridge_create_unit(unsigned long unit_type);
	int grpc_bridge_move(unsigned long unit_id, unsigned short x, unsigned short y);
	int grpc_bridge_attack(unsigned long unit_id, unsigned long target_id);
	int grpc_bridge_transfer_gems(unsigned long source_id, unsigned short x, unsigned short y, unsigned long amount);
	int grpc_bridge_build(unsigned long unit_id, unsigned short x, unsigned short y);

	// ── Debug data (unary RPC) ───────────────────────────────────────────

	/// Send accumulated debug entries to the server.
	/// entries/count come from the internal debug_data global.
	int grpc_bridge_send_debug_data(void);

	// ── Queries (unary RPCs) ─────────────────────────────────────────────
	// Returned t_obj* are owned by the per-tick cache. Do NOT free them.
	// The cache is cleared at the start of each tick.

	t_obj *grpc_bridge_get_object_by_id(unsigned long id);
	t_obj *grpc_bridge_get_object_at_pos(unsigned short x, unsigned short y);
	t_obj *grpc_bridge_get_my_core(void);
	t_obj **grpc_bridge_get_my_units(void);
	t_obj **grpc_bridge_get_all_objects(void);
	unsigned long grpc_bridge_get_tick(void);

	/// Clear the per-tick object cache. Called at tick boundary.
	void grpc_bridge_cache_clear(void);

	// ── Config (received during login, stored locally) ───────────────────

	/// Parse the protobuf config into the global game.config struct.
	/// Called internally by grpc_bridge_login.
	// (no public API needed — config is in game.config)

#ifdef __cplusplus
}
#endif

#endif // GRPC_BRIDGE_H
