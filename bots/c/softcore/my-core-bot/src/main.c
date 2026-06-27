#include "bot.h"

#include <stdlib.h>
#include <time.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	srand((unsigned int)time(NULL));
	return core_startGame("Icon Test Bot", argc, argv, ft_on_tick, false);
}

static void create_random_icon_test_unit(void)
{
	int icon = rand() % 7;

	switch (icon)
	{
	case 0:
		// sword: unit_attack is prioritized
		core_action_createUnit(NULL, "unit_attack", "health", "speed", NULL);
		break;

	case 1:
		// axe: core_attack is prioritized
		core_action_createUnit(NULL, "core_attack", "health", "speed", NULL);
		break;

	case 2:
		// pickaxe: mining is prioritized
		core_action_createUnit(NULL, "mining", "health", "speed", NULL);
		break;

	case 3:
		// shield: armor is prioritized
		core_action_createUnit(NULL, "armor", "health", "speed", NULL);
		break;

	case 4:
		// heart: no prioritized components, health is most common
		core_action_createUnit(NULL, "health", "health", "health", "speed", "gem_carrying_efficiency", NULL);
		break;

	case 5:
		// lightning: no prioritized components, speed is most common
		core_action_createUnit(NULL, "speed", "speed", "speed", "health", "gem_carrying_efficiency", NULL);
		break;

	case 6:
		// bag: no prioritized components, bag component is most common
		core_action_createUnit(NULL, "gem_carrying_efficiency", "gem_carrying_efficiency", "gem_carrying_capacity",
							   "health", "speed", NULL);
		break;
	}
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [ICON TEST TICK %ld]\n", tick);

	create_random_icon_test_unit();

	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
	{
		core_action_pathfind(units[i], ft_get_core_opponent()->pos);

		core_debug_addObjectInfo(units[i],
								 "Icon test unit. Components should determine this unit's visualizer icon.\n");
	}

	free(units);
}
