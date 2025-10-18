#include "bot.h"

// pathfinding is super basic, but you can start using this if you want to

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// ---------- COST FUNCTION ----------
static unsigned long movement_cost_at(t_pos pos)
{
	t_obj *any_obj = core_get_obj_from_pos(pos);
	if (!any_obj)
		return 1; // empty tile
	else
	{
		switch (any_obj->type)
		{
		case OBJ_WALL:
		case OBJ_CORE:
		case OBJ_DEPOSIT:
			return ULONG_MAX; // impassable
		case OBJ_UNIT:
		case OBJ_GEM_PILE:
		case OBJ_BOMB:
			return 2; // passable, but costs more
		default:
			return 1; // unknown object, assume passable
		}
	}
}

// ---------- HELPERS ----------
static inline size_t pos_to_index(t_pos p, unsigned long grid_size)
{
	return (size_t)p.y * (size_t)grid_size + (size_t)p.x;
}

static inline t_pos index_to_pos(size_t idx, unsigned long grid_size)
{
	t_pos p;
	p.x = (unsigned short)(idx % grid_size);
	p.y = (unsigned short)(idx / grid_size);
	return p;
}

static inline bool in_bounds_int(int x, int y, unsigned long grid)
{
	return x >= 0 && y >= 0 && (unsigned long)x < grid && (unsigned long)y < grid;
}

static inline bool pos_in_bounds(t_pos p, unsigned long grid)
{
	return p.x < grid && p.y < grid;
}

// ---------- MAIN: DIJKSTRA NEXT STEP ----------
// Returns the next step towards target, or start if no path found / invalid input.
t_pos pathfind_next_step_dijkstra(t_pos start, t_pos target)
{
	const unsigned long grid = game.config.gridSize;
	const size_t total = (size_t)grid * (size_t)grid;

	if (grid == 0 || !pos_in_bounds(start, grid) || !pos_in_bounds(target, grid)) return start;

	if (start.x == target.x && start.y == target.y) return start;

	// Setup
	unsigned long *distance = malloc(total * sizeof(unsigned long));
	bool *visited = malloc(total * sizeof(bool));
	size_t *parent = malloc(total * sizeof(size_t));
	if (!distance || !visited || !parent)
	{
		free(distance);
		free(visited);
		free(parent);
		return start;
	}

	for (size_t i = 0; i < total; ++i)
	{
		distance[i] = ULONG_MAX;
		visited[i] = false;
		parent[i] = (size_t)(-1);
	}

	const size_t start_idx = pos_to_index(start, grid);
	const size_t target_idx = pos_to_index(target, grid);

	distance[start_idx] = 0;

	// Dijkstra (naive scan for smallest distance)
	while (true)
	{
		size_t current = (size_t)(-1);
		unsigned long best_dist = ULONG_MAX;

		for (size_t i = 0; i < total; ++i)
			if (!visited[i] && distance[i] < best_dist) best_dist = distance[i], current = i;

		if (current == (size_t)(-1) || best_dist == ULONG_MAX) break;
		if (current == target_idx) break;

		visited[current] = true;

		t_pos cur_pos = index_to_pos(current, grid);
		const int dx[4] = {1, -1, 0, 0};
		const int dy[4] = {0, 0, 1, -1};

		for (int dir = 0; dir < 4; ++dir)
		{
			const int nx = (int)cur_pos.x + dx[dir];
			const int ny = (int)cur_pos.y + dy[dir];
			if (!in_bounds_int(nx, ny, grid)) continue;

			t_pos neighbor = {.x = (unsigned short)nx, .y = (unsigned short)ny};
			size_t n_idx = pos_to_index(neighbor, grid);
			if (visited[n_idx]) continue;

			unsigned long step_cost = movement_cost_at(neighbor);
			if (neighbor.x == target.x && neighbor.y == target.y) step_cost = 1;
			if (step_cost == ULONG_MAX) continue;

			if (distance[current] + step_cost < distance[n_idx])
			{
				distance[n_idx] = distance[current] + step_cost;
				parent[n_idx] = current;
			}
		}
	}

	// Decide which goal to reconstruct to:
	// Prefer the real target; if unreachable, choose the best adjacent tile to the target.
	size_t goal_idx = target_idx;

	if (distance[target_idx] == ULONG_MAX)
	{
		unsigned long best = ULONG_MAX;
		size_t best_idx = (size_t)(-1);

		const int dx[4] = {1, -1, 0, 0};
		const int dy[4] = {0, 0, 1, -1};

		for (int dir = 0; dir < 4; ++dir)
		{
			int nx = (int)target.x + dx[dir];
			int ny = (int)target.y + dy[dir];
			if (!in_bounds_int(nx, ny, grid)) continue;

			t_pos adj = {.x = (unsigned short)nx, .y = (unsigned short)ny};
			size_t adj_idx = pos_to_index(adj, grid);

			// Only consider reachable tiles (distance != inf)
			if (distance[adj_idx] != ULONG_MAX && distance[adj_idx] < best)
			{
				best = distance[adj_idx];
				best_idx = adj_idx;
			}
		}

		if (best_idx != (size_t)(-1)) goal_idx = best_idx; // walk next to target
	}

	// Reconstruct ONE step from start -> goal_idx
	t_pos next_step = start;
	if (distance[goal_idx] != ULONG_MAX)
	{
		size_t cur = goal_idx;
		size_t prev = (size_t)(-1);

		while (cur != start_idx && parent[cur] != (size_t)(-1))
		{
			prev = cur;
			cur = parent[cur];
		}

		if (cur == start_idx && prev != (size_t)(-1)) next_step = index_to_pos(prev, grid);
	}

	free(distance);
	free(visited);
	free(parent);
	return next_step;
}
