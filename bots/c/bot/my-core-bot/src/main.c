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
		core_action_createUnit("Test unit", "unit_attack", "health", "speed", NULL);
		break;

	case 1:
		// axe: core_attack is prioritized
		core_action_createUnit("Test unit", "core_attack", "health", "speed", NULL);
		break;

	case 2:
		// pickaxe: mining is prioritized
		core_action_createUnit("Test unit", "mining", "health", "speed", NULL);
		break;

	case 3:
		// shield: armor is prioritized
		core_action_createUnit("Test unit", "armor", "health", "speed", NULL);
		break;

	case 4:
		// heart: no prioritized components, health is most common
		core_action_createUnit("Test unit", "health", "health", "health", "speed", "gem_carrying_efficiency", NULL);
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

static int get_travel_weight(t_pos pos, const t_obj *unit)
{
	t_obj *obstacle = core_get_obj_from_pos(pos);
	if (obstacle && obstacle->type == OBJ_UNIT && obstacle->s_unit.team_id == unit->s_unit.team_id) return -1;
	if (obstacle && obstacle->type == OBJ_CORE && obstacle->s_core.team_id == unit->s_unit.team_id) return -1;
	return obstacle ? 10 : 1;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [ICON TEST TICK %ld]\n", tick);

	create_random_icon_test_unit();

	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
	{
		core_action_travel(units[i], ft_get_core_opponent()->pos, get_travel_weight);

		core_debug_addObjectInfo(units[i],
								 "Icon test unit. Components should determine this unit's visualizer icon.\n");
	}

	free(units);
}
