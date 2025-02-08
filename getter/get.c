#include "con_lib.h"

t_obj	*ft_get_obj_from_id(unsigned long id)
{
	t_obj	**objs;
	int		i;

	for (int obj_type = 0; obj_type < 3; obj_type++)
	{
		switch (obj_type)
		{
			case 0:
				objs = game.units;
				break;
			case 1:
				objs = game.cores;
				break;
			case 2:
				objs = game.resources;
				break;
		}

		i = 0;
		while (objs[i] != NULL)
		{
			if (objs[i]->id == id)
				return (objs[i]);
			i++;
		}
	}

	return (NULL);
}

t_obj *ft_get_obj_at_pos(t_pos pos)
{
	t_obj	**objs;
	int		i;

	for (int obj_type = 0; obj_type < 3; obj_type++)
	{
		switch (obj_type)
		{
			case 0:
				objs = game.units;
				break;
			case 1:
				objs = game.cores;
				break;
			case 2:
				objs = game.resources;
				break;
		}

		i = 0;
		while (objs && objs[i] != NULL)
		{
			if (objs[i]->pos.x == pos.x && objs[i]->pos.y == pos.y)
				return (objs[i]);
			i++;
		}
	}

	return (NULL);
}
