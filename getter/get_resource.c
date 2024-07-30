#include "con_lib.h"

t_obj	*ft_get_nearest_resource(t_obj *unit)
{
	int		ind = 0;
	long	min_dist = __LONG_MAX__;
	long	dist;
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
