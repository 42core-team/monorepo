#include "con_lib.h"

t_obj	*ft_get_nearest_resource(t_obj *unit)
{
	int	ind = 0;
	int	dist = 0;
	int	min_dist = 0;
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
