#include "core_lib.h"
#include "core_lib_internal.h"

t_obj *core_get_obj_from_id(unsigned long id)
{
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->id == id)
				return (game.objects[i]);
	return (NULL);
}

t_obj *core_get_obj_from_pos(t_pos pos)
{
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->pos.x == pos.x && game.objects[i]->pos.y == pos.y && game.objects[i]->state == STATE_ALIVE)
				return (game.objects[i]);
	return (NULL);
}

// -

t_unit_config *core_get_unitConfig(t_unit_type unit_type)
{
	int ind = 0;

	while (game.config.units[ind] != NULL)
	{
		if (game.config.units[ind]->unit_type == unit_type)
			return (game.config.units[ind]);
		ind++;
	}

	return (NULL);
}

// -

t_obj **core_get_objs_filter(bool (*condition)(const t_obj *))
{
	int count = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (!condition || condition(game.objects[i]))
			count++;
	}

	if (count == 0)
		return (NULL);

	t_obj **result = malloc(sizeof(t_obj *) * (count + 1));
	if (!result)
		return (NULL);

	int index = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (!condition || condition(game.objects[i]))
			result[index++] = game.objects[i];
	}
	result[index] = NULL;

	return result;
}

t_obj *core_get_obj_filter_first(bool (*condition)(const t_obj *))
{
	t_obj **objects = core_get_objs_filter(condition);
	if (!objects || !objects[0])
		return NULL;
	t_obj *first = objects[0];
	free(objects);
	return first;
}

t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *))
{
	t_obj **objects = core_get_objs_filter(condition);
	if (!objects || !objects[0])
		return NULL;

	t_obj *nearest = NULL;
	unsigned int min_distance = -1;
	for (int i = 0; objects[i] != NULL; i++)
	{
		unsigned int distance = core_internal_util_distance(pos, objects[i]->pos);
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
	if (!objects)
		return 0;

	for (int i = 0; objects[i] != NULL; i++)
		count++;

	free(objects);
	return count;
}
