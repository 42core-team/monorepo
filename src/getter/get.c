#include "con_lib.h"

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
			if (game.objects[i]->pos.x == pos.x && game.objects[i]->pos.y == pos.y)
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

t_obj **core_get_obj_customCondition(bool (*condition)(t_obj *))
{
	if (!condition)
		return (NULL);

	int count = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (condition(game.objects[i]))
			count++;
	}

	t_obj **result = malloc(sizeof(t_obj *) * (count + 1));
	if (!result)
		return (NULL);

	int index = 0;
	for (int i = 0; game.objects && game.objects[i] != NULL; i++)
	{
		if (condition(game.objects[i]))
			result[index++] = game.objects[i];
	}
	result[index] = NULL;

	return result;
}

t_obj *core_get_obj_customCondition_first(bool (*condition)(t_obj *))
{
	t_obj **objects = core_get_obj_customCondition(condition);
	if (!objects || !objects[0])
		return NULL;
	t_obj *first = objects[0];
	free(objects);
	return first;
}

t_obj *core_get_obj_customCondition_nearest(t_pos pos, bool (*condition)(t_obj *))
{
	t_obj **objects = core_get_obj_customCondition(condition);
	if (!objects || !objects[0])
		return NULL;

	t_obj *nearest = NULL;
	unsigned int min_distance = -1;
	for (int i = 0; objects[i] != NULL; i++)
	{
		unsigned int distance = core_util_distance(pos, objects[i]->pos);
		if (distance < min_distance)
		{
			min_distance = distance;
			nearest = objects[i];
		}
	}
	free(objects);
	return nearest;
}
