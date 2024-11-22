#include "con_lib.h"

t_team	*ft_get_my_team(void)
{
	int ind = 0;

	if (!game.teams)
		return (NULL);

	while (game.teams[ind] != NULL)
	{
		if (game.teams[ind]->id == game.my_team_id)
			return (game.teams[ind]);
		ind++;
	}

	return (NULL);
}

t_team	*ft_get_first_opponent_team(void)
{
	int ind = 0;

	if (!game.teams)
		return (NULL);

	while (game.teams[ind] != NULL)
	{
		if (game.teams[ind]->id != game.my_team_id)
			return (game.teams[ind]);
		ind++;
	}

	return (NULL);
}
