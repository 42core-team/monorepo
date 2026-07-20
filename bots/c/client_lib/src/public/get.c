#include "core_lib.h"
#include "core_lib_internal.h"

#include <stdint.h>

static t_obj **core_static_objPositionIndex = NULL;
static size_t core_static_objPositionIndexSize = 0;

void core_internal_objPositionIndex_rebuild(void)
{
	size_t grid_size = game.grid_size;
	size_t index_size = grid_size * grid_size;
	if (index_size == 0 || index_size > SIZE_MAX / sizeof(*core_static_objPositionIndex))
	{
		core_internal_objPositionIndex_free();
		return;
	}

	if (index_size != core_static_objPositionIndexSize)
	{
		t_obj **index = realloc(core_static_objPositionIndex, index_size * sizeof(*index));
		if (!index)
		{
			core_internal_objPositionIndex_free();
			return;
		}
		core_static_objPositionIndex = index;
		core_static_objPositionIndexSize = index_size;
	}
	memset(core_static_objPositionIndex, 0, index_size * sizeof(*core_static_objPositionIndex));

	for (size_t i = 0; game.objects && game.objects[i]; i++)
	{
		t_obj *obj = game.objects[i];
		if (!core_internal_isPosValid(obj->pos)) continue;
		size_t index = (size_t)obj->pos.y * grid_size + obj->pos.x;
		if (!core_static_objPositionIndex[index]) core_static_objPositionIndex[index] = obj;
	}
}
void core_internal_objPositionIndex_free(void)
{
	free(core_static_objPositionIndex);
	core_static_objPositionIndex = NULL;
	core_static_objPositionIndexSize = 0;
}

// -

t_obj *core_get_obj_from_id(unsigned long id)
{
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->id == id) return (game.objects[i]);
	return (NULL);
}

t_obj *core_get_obj_from_pos(t_pos pos)
{
	if (!core_internal_isPosValid(pos)) return NULL;
	if (core_static_objPositionIndex) return core_static_objPositionIndex[(size_t)pos.y * game.grid_size + pos.x];
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->pos.x == pos.x && game.objects[i]->pos.y == pos.y) return (game.objects[i]);
	return (NULL);
}

// -

static bool core_static_isOwnUnitByName(const t_obj *obj, const char *name)
{
	return obj->type == OBJ_UNIT && obj->s_unit.team_id == game.my_team_id && obj->s_unit.name &&
		   strcmp(obj->s_unit.name, name) == 0;
}
unsigned int core_get_units_byName_count(const char *name)
{
	if (!name) return 0;

	unsigned int count = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		t_obj *obj = game.objects[i];
		if (core_static_isOwnUnitByName(obj, name)) count++;
	}
	return count;
}
t_obj **core_get_units_byName(const char *name)
{
	unsigned int count = core_get_units_byName_count(name);

	if (count == 0) return NULL;

	t_obj **result = malloc(sizeof(t_obj *) * (count + 1));
	if (!result) return NULL;

	unsigned int index = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		t_obj *obj = game.objects[i];
		if (core_static_isOwnUnitByName(obj, name)) result[index++] = obj;
	}
	result[index] = NULL;

	return result;
}

// -

t_obj **core_get_objs_filter(bool (*condition)(const t_obj *))
{
	int count = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (!condition || condition(game.objects[i])) count++;
	}

	if (count == 0) return (NULL);

	t_obj **result = malloc(sizeof(t_obj *) * (count + 1));
	if (!result) return (NULL);

	int index = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (!condition || condition(game.objects[i])) result[index++] = game.objects[i];
	}
	result[index] = NULL;

	return result;
}

t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *))
{
	t_obj **objects = core_get_objs_filter(condition);
	if (!objects || !objects[0]) return NULL;

	t_obj *nearest = NULL;
	unsigned int min_distance = -1;
	for (int i = 0; objects[i] != NULL; i++)
	{
		unsigned int distance = core_internal_distance(pos, objects[i]->pos);
		if (distance < min_distance)
		{
			min_distance = distance;
			nearest = objects[i];
		}
	}
	free(objects);
	return nearest;
}

unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *))
{
	unsigned int count = 0;
	for (int i = 0; game.objects && game.objects[i]; i++)
		if (!condition || condition(game.objects[i])) count++;
	return count;
}
