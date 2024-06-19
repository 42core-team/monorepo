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
