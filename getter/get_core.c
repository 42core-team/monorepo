#include "con_lib.h"

t_obj	*ft_get_my_core(void)
{
	int	ind = 0;

	if (!game.cores)
		return (NULL);

	while (game.cores[ind] != NULL)
	{
		if (game.cores[ind]->s_core.team_id == game.my_team_id)
			return (game.cores[ind]);
		ind++;
	}

	return (NULL);
}

t_obj	*ft_get_first_opponent_core(void)
{
	int	ind = 0;

	if (!game.cores)
		return (NULL);

	while (game.cores[ind] != NULL)
	{
		if (game.cores[ind]->s_core.team_id != game.my_team_id)
			return (game.cores[ind]);
		ind++;
	}

	return (NULL);
}

t_obj	*ft_get_nearest_core(t_obj *obj)
{
	int		ind = 0;
	double	min_dist = __DBL_MAX__;
	double	dist;
	t_obj	*nearest = NULL;

	if (!game.cores)
		return (NULL);

	while (game.cores[ind] != NULL)
	{
		dist = ft_distance(obj, game.cores[ind]);
		if (dist < min_dist)
		{
			nearest = game.cores[ind];
			min_dist = dist;
		}
		ind++;
	}

	return (nearest);
}
