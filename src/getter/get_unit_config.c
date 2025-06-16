#include "con_lib.h"

t_unit_config *ft_get_unit_config(t_unit_type unit_type)
{
	int ind = 0;

	while (game.config.units[ind] != NULL)
	{
		if (game.config.units[ind]->unit_type == unit_type)
			return (game.config.units[ind]);
		ind++;
	}

	return (NULL);
}
