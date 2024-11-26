#include "con_lib.h"
#include <limits.h>
#include <stdio.h>

t_obj	**ft_get_my_units(void)
{
	if (!game.units)
		return (NULL);

	int	ind = 0;
	int	count = 0;
	while (game.units[ind] != NULL)
	{
		printf("Unit %lu, team %ld, type %ld, state %d ", game.units[ind]->id, game.units[ind]->s_unit.team_id, game.units[ind]->s_unit.type_id, game.units[ind]->state);
		if (game.units[ind]->s_unit.team_id == game.my_team_id && game.units[ind]->state == STATE_ALIVE)
		{
			count++;
			printf("is mine");
		}
		printf("\n");
		ind++;
	}

	printf("There are %d units, and %d total\n", count, ind);

	t_obj	**units = malloc((count + 1) * sizeof(t_obj *));
	ind = 0;
	count = 0;
	while (game.units[ind] != NULL)
	{
		if (game.units[ind]->s_unit.team_id == game.my_team_id && game.units[ind]->state == STATE_ALIVE)
		{
			units[count] = game.units[ind];
			count++;
		}
		ind++;
	}
	units[count] = NULL;

	return (units);
}

t_obj	**ft_get_opponent_units(void)
{
	if (!game.units)
		return (NULL);

	int	ind = 0;
	int	count = 0;
	while (game.units[ind] != NULL)
	{
		if (game.units[ind]->s_unit.team_id != game.my_team_id && game.units[ind]->state == STATE_ALIVE)
			count++;
		ind++;
	}

	t_obj	**units = malloc((count + 1) * sizeof(t_obj *));
	ind = 0;
	count = 0;
	while (game.units[ind] != NULL)
	{
		if (game.units[ind]->s_unit.team_id != game.my_team_id && game.units[ind]->state == STATE_ALIVE)
		{
			units[count] = game.units[ind];
			count++;
		}
		ind++;
	}
	units[count] = NULL;

	return (units);
}

t_obj	*ft_get_nearest_unit(t_obj *unit)
{
	if (!game.units)
		return (NULL);

	t_obj	*nearest = NULL;
	int		ind = 0;
	double	min_dist = __DBL_MAX__;
	double	dist;

	while (game.units[ind] != NULL)
	{
		dist = ft_distance(unit, game.units[ind]);
		if (dist < min_dist && game.units[ind]->state == STATE_ALIVE)
		{
			min_dist = dist;
			nearest = game.units[ind];
		}
		ind++;
	}
	return (nearest);
}

t_obj	*ft_get_nearest_opponent_unit(t_obj *unit)
{
	if (!game.units)
		return (NULL);

	t_obj	**units = ft_get_opponent_units();
	t_obj	*nearest = NULL;
	int		ind = 0;
	double	min_dist = __DBL_MAX__;
	double	dist;

	while (units[ind] != NULL)
	{
		dist = ft_distance(unit, units[ind]);
		if (dist < min_dist && game.units[ind]->state == STATE_ALIVE)
		{
			min_dist = dist;
			nearest = units[ind];
		}
		ind++;
	}

	free(units);
	return (nearest);
}
