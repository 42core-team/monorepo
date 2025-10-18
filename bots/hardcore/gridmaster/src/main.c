#include "bot.h"

#include <stdio.h>
#include <time.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("Gridmaster", argc, argv, ft_on_tick, false);
}

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
		core_action_pathfind(units[i], ft_get_core_opponent()->pos);
	free(units);
}
