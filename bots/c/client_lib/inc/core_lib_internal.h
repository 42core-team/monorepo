#ifndef CORE_LIB_INTERNAL_H
#define CORE_LIB_INTERNAL_H

#include "core_lib.h"

#ifdef __cplusplus
extern "C"
{
#endif

// ----- General

int core_internal_distance(t_pos pos1, t_pos pos2);
bool core_internal_isPosValid(t_pos pos);
void core_internal_freeGame(void);
void core_internal_freeAndExit(const char *msg, int count, ...);

// ----- Actions

typedef enum e_action_type
{
	ACTION_CREATE,
	ACTION_MOVE,
	ACTION_ATTACK,
	ACTION_TRANSFER,
	ACTION_BUILD
} t_action_type;

typedef struct s_action
{
	t_action_type type;
	union
	{
		struct
		{
			unsigned long unit_type;
		} create;
		struct
		{
			unsigned long id;
			t_pos pos;
		} move;
		struct
		{
			unsigned long id;
			unsigned long target_id;
		} attack;
		struct
		{
			unsigned long source_id;
			t_pos target_pos;
			unsigned long amount;
		} transfer;
		struct
		{
			unsigned long builder_id;
			t_pos pos;
		} build;
	} data;
} t_action;

typedef struct s_actions
{
	t_action *list;
	unsigned int count;
	unsigned int capacity;
} t_actions;

extern t_actions actions;

void core_internal_reset_actions(void);

// ----- Debug Data

typedef struct s_debug_path_node
{
	t_pos pos;
	struct s_debug_path_node *next;
} t_debug_path_node;

typedef struct s_debug_entry
{
	unsigned long object_id;
	char *info; // Accumulated info string for this object
	t_debug_path_node *path;
} t_debug_entry;

typedef struct s_debug_data
{
	t_debug_entry *entries;
	unsigned int count;
	unsigned int capacity;
} t_debug_data;

extern t_debug_data debug_data;

void core_internal_reset_debugData(void);

// ----- gRPC Bridge

#include "grpc_bridge.h"

#ifdef __cplusplus
}
#endif

#endif // CORE_LIB_INTERNAL_H
