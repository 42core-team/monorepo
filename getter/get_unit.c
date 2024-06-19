#include "con_lib.h"

t_obj	**ft_get_my_units(void)
{
	int	ind = 0;
	int	count = 0;
	while (game.units[ind].id != 0)
	{
		if (game.units->s_unit.team_id == game.my_team_id)
			count++;
		ind++;
	}

	t_obj	**units = malloc((count + 1) * sizeof(t_obj *));
	ind = 0;
	count = 0;
	while (game.units[ind].id != 0)
	{
		if (game.units[ind].s_unit.team_id == game.my_team_id)
		{
			units[count] = &game.units[ind];
			count++;
		}
		ind++;
	}
	units[count] = NULL;

	return (units);
}

t_obj	**ft_get_opponent_units(void)
{
	int	ind = 0;
	int	count = 0;
	while (game.units[ind].id != 0)
	{
		if (game.units->s_unit.team_id != game.my_team_id)
			count++;
		ind++;
	}

	t_obj	**units = malloc((count + 1) * sizeof(t_obj *));
	ind = 0;
	count = 0;
	while (game.units[ind].id != 0)
	{
		if (game.units[ind].s_unit.team_id != game.my_team_id)
		{
			units[count] = &game.units[ind];
			count++;
		}
		ind++;
	}
	units[count] = NULL;

	return (units);
}

t_obj	*ft_get_nearest_unit(t_obj *unit)
{
	t_obj	**units = ft_get_opponent_units();
	t_obj	*nearest = NULL;
	int	ind = 0;
	int	dist = -1;
	while (units[ind] != NULL)
	{
		int tmp = ft_distance(unit, units[ind]);
		if (tmp < dist)
		{
			dist = tmp;
			nearest = units[ind];
		}
		ind++;
	}

	free(units);
	return (nearest);
}
