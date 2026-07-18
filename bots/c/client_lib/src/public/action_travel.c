#include "core_lib.h"
#include "core_lib_internal.h"

#include <stdint.h>

#define TRAVEL_CAN_REMOVE 1
#define TRAVEL_BLOCKED 2

typedef struct s_travel_node
{
	int64_t distance;
	uint32_t first_step;
	uint32_t heap_pos;
	uint32_t distance_generation;
	int weight;
	unsigned char flags;
} t_travel_node;

typedef struct s_travel_workspace
{
	t_travel_node *nodes;
	uint32_t *heap;
	size_t capacity;
	uint32_t generation;
	bool busy;
} t_travel_workspace;

static t_travel_workspace core_static_travelWorkspace = {0};

static bool core_static_travelWorkspace_reserve(t_travel_workspace *workspace, size_t count)
{
	if (count <= workspace->capacity) return true;

	size_t old_capacity = workspace->capacity;
	t_travel_node *nodes = realloc(workspace->nodes, count * sizeof(*nodes));
	if (!nodes) return false;
	workspace->nodes = nodes;
	memset(workspace->nodes + old_capacity, 0, (count - old_capacity) * sizeof(*workspace->nodes));

	uint32_t *heap = realloc(workspace->heap, count * sizeof(*heap));
	if (!heap) return false;
	workspace->heap = heap;
	workspace->capacity = count;
	return true;
}

static uint32_t core_static_travelWorkspace_nextGeneration(t_travel_workspace *workspace)
{
	workspace->generation++;
	if (workspace->generation == 0)
	{
		for (size_t i = 0; i < workspace->capacity; i++)
			workspace->nodes[i].distance_generation = 0;
		workspace->generation = 1;
	}
	return workspace->generation;
}

static t_travel_node *core_static_travelNode_get(t_travel_workspace *workspace, uint32_t index, uint32_t generation,
												 uint32_t sentinel)
{
	t_travel_node *node = &workspace->nodes[index];
	if (node->distance_generation != generation)
	{
		node->distance = INT64_MAX;
		node->first_step = sentinel;
		node->heap_pos = sentinel;
		node->distance_generation = generation;
	}
	return node;
}

static void core_static_travelHeap_swap(uint32_t *heap, t_travel_node *nodes, size_t first, size_t second)
{
	uint32_t tmp = heap[first];
	heap[first] = heap[second];
	heap[second] = tmp;
	nodes[heap[first]].heap_pos = (uint32_t)first;
	nodes[heap[second]].heap_pos = (uint32_t)second;
}

static void core_static_travelHeap_siftUp(uint32_t *heap, t_travel_node *nodes, size_t pos)
{
	while (pos > 0)
	{
		size_t parent = (pos - 1) / 2;
		if (nodes[heap[parent]].distance <= nodes[heap[pos]].distance) return;
		core_static_travelHeap_swap(heap, nodes, parent, pos);
		pos = parent;
	}
}

static void core_static_travelHeap_siftDown(uint32_t *heap, t_travel_node *nodes, size_t count)
{
	size_t pos = 0;
	while (pos * 2 + 1 < count)
	{
		size_t child = pos * 2 + 1;
		if (child + 1 < count && nodes[heap[child + 1]].distance < nodes[heap[child]].distance) child++;
		if (nodes[heap[pos]].distance <= nodes[heap[child]].distance) return;
		core_static_travelHeap_swap(heap, nodes, pos, child);
		pos = child;
	}
}

static uint32_t core_static_travelHeap_pop(uint32_t *heap, t_travel_node *nodes, size_t *count, uint32_t sentinel)
{
	uint32_t result = heap[0];
	(*count)--;
	if (*count > 0)
	{
		heap[0] = heap[*count];
		nodes[heap[0]].heap_pos = 0;
		core_static_travelHeap_siftDown(heap, nodes, *count);
	}
	nodes[result].heap_pos = sentinel;
	return result;
}

static size_t core_static_travel_getNeighbors(uint32_t index, size_t grid_size, uint32_t neighbors[4])
{
	size_t count = 0;
	size_t x = index % grid_size;
	size_t y = index / grid_size;
	if (x > 0) neighbors[count++] = index - 1;
	if (x + 1 < grid_size) neighbors[count++] = (uint32_t)(index + 1);
	if (y > 0) neighbors[count++] = (uint32_t)(index - grid_size);
	if (y + 1 < grid_size) neighbors[count++] = (uint32_t)(index + grid_size);
	return count;
}

static bool core_static_travel_addWeight(int64_t distance, int weight, int64_t *result)
{
	if (weight > 0 && distance > INT64_MAX - weight) return false;
	if (weight < 0 && distance < INT64_MIN - (int64_t)weight) return false;
	*result = distance + weight;
	return true;
}

static bool core_static_travel_prepareSurfaces(t_travel_workspace *workspace, size_t count, size_t grid_size,
											   uint32_t start, const t_obj *unit,
											   t_travel_surface (*get_surface)(t_pos, const t_obj *))
{
	bool has_negative_weight = false;
	for (uint32_t i = 0; i < count; i++)
	{
		t_pos position = {(unsigned short)(i % grid_size), (unsigned short)(i / grid_size)};
		t_travel_surface surface = get_surface(position, unit);
		t_obj *obstacle = core_get_obj_from_pos(position);
		t_travel_node *node = &workspace->nodes[i];
		node->weight = surface.weight;
		node->flags = surface.can_remove ? TRAVEL_CAN_REMOVE : 0;
		if (i != start && obstacle && !surface.can_remove) node->flags |= TRAVEL_BLOCKED;
		if (!(node->flags & TRAVEL_BLOCKED) && surface.weight < 0) has_negative_weight = true;
	}
	return has_negative_weight;
}

static void core_static_travel_dijkstra(t_travel_workspace *workspace, size_t count, size_t grid_size,
										uint32_t generation, uint32_t start, uint32_t target)
{
	uint32_t sentinel = (uint32_t)count;
	t_travel_node *start_node = core_static_travelNode_get(workspace, start, generation, sentinel);
	start_node->distance = 0;
	start_node->first_step = start;
	start_node->heap_pos = 0;
	workspace->heap[0] = start;
	size_t heap_count = 1;

	while (heap_count > 0)
	{
		uint32_t current = core_static_travelHeap_pop(workspace->heap, workspace->nodes, &heap_count, sentinel);
		if (current == target) return;

		uint32_t neighbors[4];
		size_t neighbor_count = core_static_travel_getNeighbors(current, grid_size, neighbors);
		for (size_t i = 0; i < neighbor_count; i++)
		{
			uint32_t next = neighbors[i];
			t_travel_node *next_node = core_static_travelNode_get(workspace, next, generation, sentinel);
			if (next_node->flags & TRAVEL_BLOCKED) continue;
			if (next_node->distance != INT64_MAX && next_node->heap_pos == sentinel) continue;

			int64_t distance;
			if (!core_static_travel_addWeight(workspace->nodes[current].distance, next_node->weight, &distance))
				continue;
			if (distance >= next_node->distance) continue;
			bool unseen = next_node->distance == INT64_MAX;
			next_node->distance = distance;
			next_node->first_step = current == start ? next : workspace->nodes[current].first_step;
			if (unseen)
			{
				next_node->heap_pos = (uint32_t)heap_count;
				workspace->heap[heap_count++] = next;
			}
			core_static_travelHeap_siftUp(workspace->heap, workspace->nodes, next_node->heap_pos);
		}
	}
}

static bool core_static_travel_bellmanFord(t_travel_workspace *workspace, size_t count, size_t grid_size,
										   uint32_t generation, uint32_t start)
{
	uint32_t sentinel = (uint32_t)count;
	t_travel_node *start_node = core_static_travelNode_get(workspace, start, generation, sentinel);
	start_node->distance = 0;
	start_node->first_step = start;

	for (size_t pass = 0; pass < count; pass++)
	{
		bool changed = false;
		for (uint32_t current = 0; current < count; current++)
		{
			t_travel_node *current_node = core_static_travelNode_get(workspace, current, generation, sentinel);
			if (current_node->distance == INT64_MAX || (current_node->flags & TRAVEL_BLOCKED)) continue;

			uint32_t neighbors[4];
			size_t neighbor_count = core_static_travel_getNeighbors(current, grid_size, neighbors);
			for (size_t i = 0; i < neighbor_count; i++)
			{
				uint32_t next = neighbors[i];
				t_travel_node *next_node = core_static_travelNode_get(workspace, next, generation, sentinel);
				if (next_node->flags & TRAVEL_BLOCKED) continue;

				int64_t distance;
				if (!core_static_travel_addWeight(current_node->distance, next_node->weight, &distance)) return false;
				if (distance >= next_node->distance) continue;
				next_node->distance = distance;
				next_node->first_step = current == start ? next : current_node->first_step;
				changed = true;
			}
		}
		if (!changed) return true;
		if (pass + 1 == count) return false;
	}
	return true;
}

static uint32_t core_static_travel_getClosest(t_travel_workspace *workspace, size_t count, size_t grid_size,
											  uint32_t generation, uint32_t start, uint32_t target, t_pos target_pos)
{
	uint32_t sentinel = (uint32_t)count;
	t_travel_node *target_node = core_static_travelNode_get(workspace, target, generation, sentinel);
	if (target_node->distance != INT64_MAX) return target;

	uint32_t closest = start;
	t_pos start_pos = {(unsigned short)(start % grid_size), (unsigned short)(start / grid_size)};
	unsigned int closest_distance = core_internal_distance(start_pos, target_pos);
	for (uint32_t i = 0; i < count; i++)
	{
		t_travel_node *node = core_static_travelNode_get(workspace, i, generation, sentinel);
		if (node->distance == INT64_MAX || node->first_step == sentinel) continue;
		t_pos position = {(unsigned short)(i % grid_size), (unsigned short)(i / grid_size)};
		unsigned int distance = core_internal_distance(position, target_pos);
		if (distance < closest_distance ||
			(distance == closest_distance && node->distance < workspace->nodes[closest].distance))
		{
			closest = i;
			closest_distance = distance;
		}
	}
	return closest;
}

void core_internal_travelWorkspace_free(void)
{
	free(core_static_travelWorkspace.nodes);
	free(core_static_travelWorkspace.heap);
	core_static_travelWorkspace = (t_travel_workspace){0};
}

void core_action_travel(const t_obj *unit, t_pos pos, t_travel_surface (*get_surface)(t_pos, const t_obj *))
{
	if (!unit || unit->type != OBJ_UNIT || unit->s_unit.action_cooldown > 0 || !get_surface) return;
	if (!core_internal_isPosValid(unit->pos) || !core_internal_isPosValid(pos)) return;
	if (unit->pos.x == pos.x && unit->pos.y == pos.y) return;

	size_t grid_size = game.grid_size;
	size_t node_count = grid_size * grid_size;
	if (node_count == 0 || node_count >= UINT32_MAX || node_count > SIZE_MAX / sizeof(t_travel_node) ||
		node_count > SIZE_MAX / sizeof(uint32_t))
		return;

	t_travel_workspace local_workspace = {0};
	t_travel_workspace *workspace = core_static_travelWorkspace.busy ? &local_workspace : &core_static_travelWorkspace;
	workspace->busy = true;
	if (!core_static_travelWorkspace_reserve(workspace, node_count)) goto cleanup;

	uint32_t generation = core_static_travelWorkspace_nextGeneration(workspace);
	uint32_t start = (uint32_t)((size_t)unit->pos.y * grid_size + unit->pos.x);
	uint32_t target = (uint32_t)((size_t)pos.y * grid_size + pos.x);
	bool has_negative_weight =
			core_static_travel_prepareSurfaces(workspace, node_count, grid_size, start, unit, get_surface);
	if (has_negative_weight)
	{
		if (!core_static_travel_bellmanFord(workspace, node_count, grid_size, generation, start)) goto cleanup;
	}
	else
		core_static_travel_dijkstra(workspace, node_count, grid_size, generation, start, target);

	uint32_t destination =
			core_static_travel_getClosest(workspace, node_count, grid_size, generation, start, target, pos);
	uint32_t next = workspace->nodes[destination].first_step;
	if (destination == start || next >= node_count) goto cleanup;

	t_pos next_pos = {(unsigned short)(next % grid_size), (unsigned short)(next / grid_size)};
	t_obj *obstacle = core_get_obj_from_pos(next_pos);
	if (obstacle && workspace->nodes[next].flags & TRAVEL_CAN_REMOVE)
		core_action_attack(unit, obstacle);
	else if (!obstacle)
		core_action_move(unit, next_pos);

cleanup:
	workspace->busy = false;
	if (workspace == &local_workspace)
	{
		free(workspace->nodes);
		free(workspace->heap);
	}
}
