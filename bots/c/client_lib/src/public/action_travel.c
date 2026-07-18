#include "core_lib.h"
#include "core_lib_internal.h"

#include <stdint.h>

typedef struct s_travel_node
{
	unsigned long long distance;
	size_t first_step;
	size_t heap_pos;
	int weight;
	bool has_weight;
} t_travel_node;

static void core_static_travelHeap_swap(size_t *heap, t_travel_node *nodes, size_t first, size_t second)
{
	size_t tmp = heap[first];
	heap[first] = heap[second];
	heap[second] = tmp;
	nodes[heap[first]].heap_pos = first;
	nodes[heap[second]].heap_pos = second;
}

static void core_static_travelHeap_siftUp(size_t *heap, t_travel_node *nodes, size_t pos)
{
	while (pos > 0)
	{
		size_t parent = (pos - 1) / 2;
		if (nodes[heap[parent]].distance <= nodes[heap[pos]].distance) return;
		core_static_travelHeap_swap(heap, nodes, parent, pos);
		pos = parent;
	}
}

static void core_static_travelHeap_siftDown(size_t *heap, t_travel_node *nodes, size_t count)
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

static size_t core_static_travelHeap_pop(size_t *heap, t_travel_node *nodes, size_t *count, size_t sentinel)
{
	size_t result = heap[0];

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

void core_action_travel(const t_obj *unit, t_pos pos, int (*get_weight)(t_pos, const t_obj *))
{
	if (!unit || unit->type != OBJ_UNIT || unit->s_unit.action_cooldown > 0 || !get_weight) return;
	if (!core_internal_isPosValid(unit->pos) || !core_internal_isPosValid(pos)) return;
	if (unit->pos.x == pos.x && unit->pos.y == pos.y) return;

	size_t grid_size = game.grid_size;
	size_t node_count = grid_size * grid_size;
	if (node_count > SIZE_MAX / sizeof(t_travel_node) || node_count > SIZE_MAX / sizeof(size_t)) return;

	t_travel_node *nodes = malloc(node_count * sizeof(*nodes));
	size_t *heap = malloc(node_count * sizeof(*heap));
	if (!nodes || !heap)
	{
		free(nodes);
		free(heap);
		return;
	}

	for (size_t i = 0; i < node_count; i++)
	{
		nodes[i].distance = ULLONG_MAX;
		nodes[i].first_step = node_count;
		nodes[i].heap_pos = node_count;
		nodes[i].has_weight = false;
	}

	size_t start = (size_t)unit->pos.y * grid_size + unit->pos.x;
	size_t target = (size_t)pos.y * grid_size + pos.x;
	size_t heap_count = 1;
	nodes[start].distance = 0;
	nodes[start].first_step = start;
	nodes[start].heap_pos = 0;
	heap[0] = start;

	while (heap_count > 0)
	{
		size_t current = core_static_travelHeap_pop(heap, nodes, &heap_count, node_count);
		if (current == target) break;

		size_t neighbors[4];
		size_t neighbor_count = 0;
		size_t x = current % grid_size;
		size_t y = current / grid_size;
		if (x > 0) neighbors[neighbor_count++] = current - 1;
		if (x + 1 < grid_size) neighbors[neighbor_count++] = current + 1;
		if (y > 0) neighbors[neighbor_count++] = current - grid_size;
		if (y + 1 < grid_size) neighbors[neighbor_count++] = current + grid_size;

		for (size_t i = 0; i < neighbor_count; i++)
		{
			size_t next = neighbors[i];
			if (nodes[next].distance != ULLONG_MAX && nodes[next].heap_pos == node_count) continue;
			if (!nodes[next].has_weight)
			{
				t_pos next_pos = {(unsigned short)(next % grid_size), (unsigned short)(next / grid_size)};
				nodes[next].weight = get_weight(next_pos, unit);
				nodes[next].has_weight = true;
			}
			if (nodes[next].weight < 0) continue;
			if (nodes[current].distance > ULLONG_MAX - (unsigned int)nodes[next].weight) continue;

			unsigned long long distance = nodes[current].distance + (unsigned int)nodes[next].weight;
			if (distance >= nodes[next].distance) continue;
			bool unseen = nodes[next].distance == ULLONG_MAX;
			nodes[next].distance = distance;
			nodes[next].first_step = current == start ? next : nodes[current].first_step;

			if (unseen)
			{
				nodes[next].heap_pos = heap_count;
				heap[heap_count++] = next;
			}
			core_static_travelHeap_siftUp(heap, nodes, nodes[next].heap_pos);
		}
	}

	size_t next = nodes[target].first_step;
	free(heap);
	free(nodes);
	if (next == node_count) return;

	t_pos next_pos = {(unsigned short)(next % grid_size), (unsigned short)(next / grid_size)};
	t_obj *obstacle = core_get_obj_from_pos(next_pos);
	if (obstacle)
		core_action_attack(unit, obstacle);
	else
		core_action_move(unit, next_pos);
}
