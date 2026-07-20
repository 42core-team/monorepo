#include "core_lib.h"
#include "core_lib_internal.h"

#include <stdint.h>

#define TRAVEL_CAN_REMOVE 1
#define TRAVEL_BLOCKED 2

typedef struct s_travel_node
{
	uint64_t distance;
	uint32_t first_step;
	uint32_t heap_pos;
	unsigned int weight;
	unsigned char flags;
} t_travel_node;

typedef struct s_travel_workspace
{
	t_travel_node *nodes;
	uint32_t *heap;
	size_t capacity;
} t_travel_workspace;

static t_travel_workspace core_static_travelWorkspace = {0};

static bool core_static_travelWorkspace_reserve(t_travel_workspace *workspace, size_t count)
{
	if (count <= workspace->capacity) return true;

	t_travel_node *nodes = realloc(workspace->nodes, count * sizeof(*nodes));
	if (!nodes) return false;
	workspace->nodes = nodes;

	uint32_t *heap = realloc(workspace->heap, count * sizeof(*heap));
	if (!heap) return false;
	workspace->heap = heap;
	workspace->capacity = count;
	return true;
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

static unsigned long core_static_travel_getDamage(const t_obj *unit, const t_obj *object)
{
	unsigned long damage = 0;
	if (object->type == OBJ_CORE)
		damage = unit->s_unit.properties.damage_core;
	else if (object->type == OBJ_UNIT)
	{
		damage = unit->s_unit.properties.damage_unit;
		if (damage > 0)
		{
			int reduction = object->s_unit.properties.damage_reduction_percent;
			if (reduction < 0) reduction = 0;
			if (reduction > 100) reduction = 100;
			uint64_t reduced = ((uint64_t)damage * (unsigned int)(100 - reduction) + 50) / 100;
			damage = reduced > 0 ? (unsigned long)reduced : 1;
		}
	}
	else if (object->type == OBJ_DEPOSIT || object->type == OBJ_WALL)
		damage = unit->s_unit.properties.damage_object;
	else if (object->type == OBJ_GEM_PILE)
		damage = 1;
	return damage;
}

static bool core_static_travel_isFriendly(const t_obj *unit, const t_obj *object)
{
	return (object->type == OBJ_UNIT && object->s_unit.team_id == unit->s_unit.team_id) ||
		   (object->type == OBJ_CORE && object->s_core.team_id == unit->s_unit.team_id);
}

static unsigned int core_static_travel_defaultWeight(t_pos pos, const t_obj *unit)
{
	t_obj *object = core_get_obj_from_pos(pos);
	if (!object) return 1;
	if (core_static_travel_isFriendly(unit, object)) return CORE_TRAVEL_BLOCKED;
	if (object->type == OBJ_GEM_PILE)
	{
		unsigned long max_balance = unit->s_unit.properties.max_balance;
		if (max_balance == 0 || unit->s_unit.gems >= max_balance) return CORE_TRAVEL_BLOCKED;
	}

	unsigned long damage = core_static_travel_getDamage(unit, object);
	if (damage == 0) return CORE_TRAVEL_BLOCKED;
	uint64_t attacks = object->hp / damage + (object->hp % damage != 0);
	return attacks >= CORE_TRAVEL_BLOCKED - 1 ? CORE_TRAVEL_BLOCKED - 1 : (unsigned int)attacks + 1;
}

static void core_static_travel_prepareSurfaces(t_travel_workspace *workspace, size_t count, size_t grid_size,
											   const t_obj *unit, unsigned int (*get_weight)(t_pos, const t_obj *))
{
	uint32_t sentinel = (uint32_t)count;
	for (uint32_t i = 0; i < count; i++)
	{
		t_pos position = {(unsigned short)(i % grid_size), (unsigned short)(i / grid_size)};
		t_obj *obstacle = core_get_obj_from_pos(position);
		t_travel_node *node = &workspace->nodes[i];
		node->distance = UINT64_MAX;
		node->first_step = sentinel;
		node->heap_pos = sentinel;
		node->weight = get_weight(position, unit);
		node->flags = node->weight == CORE_TRAVEL_BLOCKED ? TRAVEL_BLOCKED : 0;
		if (obstacle && !(node->flags & TRAVEL_BLOCKED)) node->flags = TRAVEL_CAN_REMOVE;
	}
}

static void core_static_travel_dijkstra(t_travel_workspace *workspace, size_t count, size_t grid_size, uint32_t start,
										uint32_t target)
{
	uint32_t sentinel = (uint32_t)count;
	t_travel_node *start_node = &workspace->nodes[start];
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
			t_travel_node *next_node = &workspace->nodes[next];
			if (next_node->flags & TRAVEL_BLOCKED) continue;
			if (next_node->distance != UINT64_MAX && next_node->heap_pos == sentinel) continue;

			uint64_t distance = workspace->nodes[current].distance + next_node->weight;
			if (distance >= next_node->distance) continue;
			bool unseen = next_node->distance == UINT64_MAX;
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

static uint32_t core_static_travel_getClosest(t_travel_workspace *workspace, size_t count, size_t grid_size,
											  uint32_t start, uint32_t target, t_pos target_pos)
{
	if (workspace->nodes[target].distance != UINT64_MAX) return target;

	uint32_t closest = start;
	t_pos start_pos = {(unsigned short)(start % grid_size), (unsigned short)(start / grid_size)};
	unsigned int closest_distance = core_internal_distance(start_pos, target_pos);
	for (uint32_t i = 0; i < count; i++)
	{
		t_travel_node *node = &workspace->nodes[i];
		if (node->distance == UINT64_MAX) continue;
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

void core_action_travel(const t_obj *unit, t_pos pos, unsigned int (*get_weight)(t_pos, const t_obj *))
{
	if (!unit || unit->type != OBJ_UNIT || unit->s_unit.action_cooldown > 0) return;
	if (!core_internal_isPosValid(unit->pos) || !core_internal_isPosValid(pos)) return;
	if (unit->pos.x == pos.x && unit->pos.y == pos.y) return;
	if (!get_weight) get_weight = core_static_travel_defaultWeight;

	size_t grid_size = game.grid_size;
	size_t node_count = grid_size * grid_size;
	if (node_count == 0 || node_count >= UINT32_MAX || node_count > SIZE_MAX / sizeof(t_travel_node) ||
		node_count > SIZE_MAX / sizeof(uint32_t))
		return;

	t_travel_workspace *workspace = &core_static_travelWorkspace;
	if (!core_static_travelWorkspace_reserve(workspace, node_count)) return;

	uint32_t start = (uint32_t)((size_t)unit->pos.y * grid_size + unit->pos.x);
	uint32_t target = (uint32_t)((size_t)pos.y * grid_size + pos.x);
	core_static_travel_prepareSurfaces(workspace, node_count, grid_size, unit, get_weight);
	core_static_travel_dijkstra(workspace, node_count, grid_size, start, target);

	uint32_t destination = core_static_travel_getClosest(workspace, node_count, grid_size, start, target, pos);
	uint32_t next = workspace->nodes[destination].first_step;
	if (destination == start || next >= node_count) return;

	t_pos next_pos = {(unsigned short)(next % grid_size), (unsigned short)(next / grid_size)};
	t_obj *obstacle = core_get_obj_from_pos(next_pos);
	if (obstacle && workspace->nodes[next].flags & TRAVEL_CAN_REMOVE)
		core_action_attack(unit, obstacle);
	else if (!obstacle)
		core_action_move(unit, next_pos);
}
