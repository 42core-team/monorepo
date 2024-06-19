#include "con_lib.h"

t_obj	*ft_get_nearest_resource(t_obj *unit)
{
	int		ind = 0;
	double	min_dist = __DBL_MAX__;
	double	dist;
	t_obj	*nearest = NULL;

	while (game.resources[ind].id != 0)
	{
		dist = ft_distance(unit, &game.resources[ind]);
		if (dist < min_dist)
		{
			nearest = &game.resources[ind];
			min_dist = dist;
		}
		ind++;
	}

	return (nearest);
}
