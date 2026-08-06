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
	{
		if (units[i]->s_unit.action_cooldown <= 0)
			core_action_travel(units[i], target->pos, NULL);
		core_debug_addObjectInfo(
			units[i],
			"Heading for the opponent core! Use the addObjectInfo function to "
			"add debug data to objects. Look at "
			"the unit tooltip in the visualizer, there you can see this text "
			"displayed for easy debugging!");
	}

	free(units);
}
