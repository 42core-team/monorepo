#include "bot.h"

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

#define PATHFINDING_DEBUG false

int ft_util_distance(t_pos pos1, t_pos pos2);

typedef struct s_node {
	t_pos pos;
	unsigned long cost;
	bool visited;
	int prev_index;
} t_node;

static unsigned long movement_cost(t_pos pos, t_obj *unit)
{
	t_obj *obj = core_get_obj_from_pos(pos);
	if (!obj)
		return 1;
	else if (obj->type == OBJ_RESOURCE && obj->state == STATE_ALIVE)
	{
		t_unit_config *config = core_get_unitConfig(unit->s_unit.unit_type);
		if (!config || config->dmg_resource == 0)
			return ULONG_MAX / 2;
		unsigned long break_cost = (obj->hp + config->dmg_resource - 1) / config->dmg_resource;
		return break_cost + 1;
	}
	else if (obj->type == OBJ_CORE && obj->state == STATE_ALIVE)
	{
		if (ft_util_distance(pos, unit->pos) > 1)
			return 1;
		if (obj->s_core.team_id == unit->s_unit.team_id)
			return ULONG_MAX / 2;
		return 1;
	}
	else if (obj->type == OBJ_WALL && obj->state == STATE_ALIVE)
	{
		t_unit_config *config = core_get_unitConfig(unit->s_unit.unit_type);
		if (!config || config->dmg_wall == 0)
			return ULONG_MAX / 2;
		unsigned long break_cost = (obj->hp + config->dmg_wall - 1) / config->dmg_wall;
		return break_cost + 1;
	}
	else if (obj->type == OBJ_UNIT && obj->state == STATE_ALIVE)
	{
		if (obj->s_unit.team_id == unit->s_unit.team_id)
		{
			if (ft_util_distance(pos, unit->pos) > 1)
				return 1;
			return  ULONG_MAX / 2;
		}
		else
		{
			if (unit->s_unit.unit_type == UNIT_WARRIOR)
				return 1;
			else
				return ULONG_MAX / 2;
		}
	}
	// else if (obj->type == OBJ_BOMB && obj->state == STATE_ALIVE)
	// {
	// 	return ULONG_MAX / 2;
	// }
	else if (obj->type == OBJ_MONEY && obj->state == STATE_ALIVE)
	{
		if (unit->s_unit.unit_type == UNIT_CARRIER || unit->s_unit.unit_type == UNIT_MINER)
			return 1;
		else
			return ULONG_MAX / 2;
	}
	return 1;
}

t_pos *find_path(t_pos start, t_pos target, t_obj *unit, int *path_length)
{
	unsigned long width = game.config.gridSize;
	unsigned long height = game.config.gridSize;
	int total = width * height;
	t_node *nodes = malloc(total * sizeof(t_node));
	if (!nodes)
		return NULL;

	for (unsigned long y = 0; y < height; y++) {
		for (unsigned long x = 0; x < width; x++) {
			int idx = y * width + x;
			nodes[idx].pos.x = x;
			nodes[idx].pos.y = y;
			nodes[idx].cost = ULONG_MAX;
			nodes[idx].visited = false;
			nodes[idx].prev_index = -1;
		}
	}

	int start_index = start.y * width + start.x;
	nodes[start_index].cost = 0;

	while (true) {
		int current_index = -1;
		unsigned long min_cost = ULONG_MAX;
		int *candidates = malloc(total * sizeof(int));
		if (!candidates) break;
		int candidate_count = 0;
		
		for (int i = 0; i < total; i++) {
			if (!nodes[i].visited) {
				if (nodes[i].cost < min_cost) {
					min_cost = nodes[i].cost;
					candidate_count = 0;
					candidates[candidate_count++] = i;
				} else if (nodes[i].cost == min_cost) {
					candidates[candidate_count++] = i;
				}
			}
		}
		
		if (candidate_count == 0) {
			free(candidates);
			break;
		}
		current_index = candidates[rand() % candidate_count];
		free(candidates);
		
		if (nodes[current_index].pos.x == target.x && nodes[current_index].pos.y == target.y)
			break;

		nodes[current_index].visited = true;

		int directions[4][2] = { {0, -1}, {1, 0}, {0, 1}, {-1, 0} };
		for (int d = 0; d < 4; d++) {
			int nx = nodes[current_index].pos.x + directions[d][0];
			int ny = nodes[current_index].pos.y + directions[d][1];
			if (nx < 0 || ny < 0 || nx >= (int)width || ny >= (int)height)
				continue;
			int neighbor_index = ny * width + nx;
			if (nodes[neighbor_index].visited)
				continue;
			t_pos neighbor_pos = { (unsigned short)nx, (unsigned short)ny };
			unsigned long cost = movement_cost(neighbor_pos, unit);
			if (cost >= ULONG_MAX / 2)
				continue;
			unsigned long new_cost = nodes[current_index].cost + cost;
			if (new_cost < nodes[neighbor_index].cost) {
				nodes[neighbor_index].cost = new_cost;
				nodes[neighbor_index].prev_index = current_index;
			}
		}
	}

	int target_index = target.y * width + target.x;
	if (nodes[target_index].cost == ULONG_MAX) {
		free(nodes);
		*path_length = 0;
		return NULL;
	}

	int count = 0;
	int idx = target_index;
	while (idx != -1) {
		count++;
		idx = nodes[idx].prev_index;
	}

	t_pos *path = malloc(count * sizeof(t_pos));
	if (!path) {
		free(nodes);
		*path_length = 0;
		return NULL;
	}
	idx = target_index;
	for (int i = count - 1; i >= 0; i--) {
		path[i] = nodes[idx].pos;
		idx = nodes[idx].prev_index;
	}
	*path_length = count;
	free(nodes);
	return path;
}

void ft_travel_to_pos(t_obj *unit, t_pos pos)
{
	bool biggestAxisX = abs(unit->pos.x - pos.x) > abs(unit->pos.y - pos.y);
	if (biggestAxisX)
	{
		if (unit->pos.x < pos.x)
			core_action_move(unit, (t_pos){unit->pos.x - 1, unit->pos.y});
		else if (unit->pos.x > pos.x)
			core_action_move(unit, (t_pos){unit->pos.x + 1, unit->pos.y});
	}
	else
	{
		if (unit->pos.y < pos.y)
			core_action_move(unit, (t_pos){unit->pos.x, unit->pos.y + 1});
		else if (unit->pos.y > pos.y)
			core_action_move(unit, (t_pos){unit->pos.x, unit->pos.y - 1});
	}
}

void move_unit_to(t_obj *unit, t_pos target)
{
	t_pos *path;
	int path_length;

	#if PATHFINDING_DEBUG
		printf("[PTHF] moving unit %lu from (%d,%d) to (%d,%d)\n",
			unit->id, (int)unit->pos.x, (int)unit->pos.y,
			(int)target.x, (int)target.y);
	#endif

	path = find_path(unit->pos, target, unit, &path_length);
	if (!path) {
		#if PATHFINDING_DEBUG
			printf("[PTHF] no path found for unit %lu from (%d,%d) to (%d,%d)\n",
				unit->id, (int)unit->pos.x, (int)unit->pos.y,
				(int)target.x, (int)target.y);
		#endif
		return;
	}

	#if PATHFINDING_DEBUG
		printf("[PTHF] path found for unit %lu from (%d,%d) to (%d,%d), length=%d\n",
			unit->id, (int)unit->pos.x, (int)unit->pos.y,
			(int)target.x, (int)target.y, path_length);
		for (int i = 0; i < path_length; i++) {
			printf("(%d,%d)%s",
					(int)path[i].x, (int)path[i].y,
					(i + 1 < path_length) ? " → " : "\n");
		}
	#endif

	if (path_length > 1)
	{
		t_pos  next = path[1];
		t_obj *obj  = core_get_obj_from_pos(next);

		if (obj && obj->type == OBJ_UNIT &&
			obj->s_unit.team_id == unit->s_unit.team_id)
		{
			free(path);
			#if PATHFINDING_DEBUG
				printf("[PTHF] found friendly unit at (%d,%d), skipping move\n",
					(int)obj->pos.x, (int)obj->pos.y);
			#endif
			return;
		}
		if (obj && obj->type == OBJ_CORE &&
			obj->s_core.team_id == unit->s_unit.team_id)
		{
			free(path);
			#if PATHFINDING_DEBUG
				printf("[PTHF] found friendly core at (%d,%d), skipping move\n",
					(int)obj->pos.x, (int)obj->pos.y);
			#endif
			return;
		}

		#if PATHFINDING_DEBUG
		printf("[PTHF] moving unit to (%d,%d)\n", (int)next.x, (int)next.y);
		#endif
		if (!obj || obj->type == OBJ_MONEY)
			core_action_move(unit, next);
		else
			core_action_attack(unit, next);
	}
	else
	{
		core_action_move(unit, target);
	}

	free(path);
}
