#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

// NOT SURE HOW TO GET STARTED?
/**
 * Perform per-tick bot actions: print a tick header, spawn a warrior unit,
 * command each owned unit to pathfind to the opponent core, and emit debug info.
 *
 * @param tick Current tick number provided by the game engine.
 */

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
	{
		core_action_pathfind(units[i], ft_get_core_opponent()->pos);
		core_debug_addObjectInfo(units[i], "I am a warrior! 🗡️ - I am heading for the opponent core at [%d,%d]! 🏰\n",
								 ft_get_core_opponent()->pos.x, ft_get_core_opponent()->pos.y);
	}
	free(units);
}