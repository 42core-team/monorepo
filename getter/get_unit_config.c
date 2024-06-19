#include "con_lib.h"

t_unit_config	*ft_get_unit_config(t_unit_type type)
{
	int	ind = 0;

	while (game.config.units[ind].type_id != 0)
	{
		if (game.config.units[ind].type_id == type)
			return (&game.config.units[ind]);
		ind++;
	}

	return (NULL);
}
