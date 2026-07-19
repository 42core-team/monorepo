#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}

void ft_on_tick(unsigned long tick)
{
	(void)tick;
	core_action_createUnit("Warrior", "combat", NULL);

	t_obj **units = ft_get_units_own();
	t_obj *target = ft_get_core_opponent();
	for (int i = 0; target && units && units[i]; i++)
		ft_travel_attack(units[i], target->pos);

	free(units);
}
