#include "con_lib.h"

t_obj	*ft_get_my_core(void)
{
	int	ind = 0;

	while (game.cores[ind].id != 0)
	{
		if (game.cores[ind].s_core.team_id == game.my_team_id)
			return (&game.cores[ind]);
		ind++;
	}
}

t_obj	*ft_get_first_opponent_core(void)
{
	int	ind = 0;

	while (game.cores[ind].id != 0)
	{
		if (game.cores[ind].s_core.team_id != game.my_team_id)
			return (&game.cores[ind]);
		ind++;
	}
}
