#include "con_lib.h"

t_obj	*ft_get_obj_from_id(unsigned long id)
{
	t_obj	*objs;
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
		while (objs[i].id != 0)
		{
			if (objs[i].id == id)
				return (&objs[i]);
			i++;
		}
	}

	return (NULL);
}
