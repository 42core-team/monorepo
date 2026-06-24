#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

// NOT SURE HOW TO GET STARTED?
// --> Check out the wiki coregame.sh/wiki/ with a helpful "My first Core bot" beginners guide!

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_ACID_WARRIOR);

	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
	{
		core_action_pathfind(units[i], ft_get_core_opponent()->pos);

		// this message will be displayed in the units tooltip in the visualizer,
		// which can be helpful for debugging and understanding what your bot is doing.
		core_debug_addObjectInfo(units[i],
								 "I am an acid warrior! 🗡️🧬 - I am heading for the opponent core at [%d,%d]! 🏰\n",
								 ft_get_core_opponent()->pos.x, ft_get_core_opponent()->pos.y);
	}
	free(units);
}
