#include "con_lib.h"

t_obj	*ft_get_obj_from_id(unsigned long id)
{
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->id == id)
				return (game.objects[i]);
	return (NULL);
}

t_obj *ft_get_obj_at_pos(t_pos pos)
{
	if (game.objects && game.objects[0] != NULL)
		for (int i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->pos.x == pos.x && game.objects[i]->pos.y == pos.y)
				return (game.objects[i]);
	return (NULL);
}
