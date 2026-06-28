#include "core_lib.h"
#include "core_lib_internal.h"

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
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->pos.x == pos.x && game.objects[i]->pos.y == pos.y) return (game.objects[i]);
	return (NULL);
}

// -

t_obj **core_get_units_by_name(const char *name)
{
	if (!name) return NULL;

	int count = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		t_obj *obj = game.objects[i];
		if (obj->type == OBJ_UNIT && obj->s_unit.name && strcmp(obj->s_unit.name, name) == 0) count++;
	}

	if (count == 0) return NULL;

	t_obj **result = malloc(sizeof(t_obj *) * (count + 1));
	if (!result) return NULL;

	int index = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		t_obj *obj = game.objects[i];
		if (obj->type == OBJ_UNIT && obj->s_unit.name && strcmp(obj->s_unit.name, name) == 0) result[index++] = obj;
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
	t_obj **objects = core_get_objs_filter(condition);
	if (!objects) return 0;

	for (int i = 0; objects[i] != NULL; i++)
		count++;

	free(objects);
	return count;
}
