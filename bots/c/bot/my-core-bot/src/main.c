#include "bot.h"

#include <stdlib.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("My CORE Bot", argc, argv, ft_on_tick, false);
}

static t_travel_surface get_travel_surface(t_pos pos, const t_obj *unit)
{
	t_obj *obstacle = core_get_obj_from_pos(pos);
	bool friendly = obstacle && ((obstacle->type == OBJ_UNIT && obstacle->s_unit.team_id == unit->s_unit.team_id) ||
								 (obstacle->type == OBJ_CORE && obstacle->s_core.team_id == unit->s_unit.team_id));
	return (t_travel_surface){obstacle ? 10 : 1, obstacle && !friendly};
}

void ft_on_tick(unsigned long tick)
{
	(void)tick;
	core_action_createUnit("Warrior", "combat", NULL);

	t_obj **units = ft_get_units_own();
	t_obj *target = ft_get_core_opponent();
	for (int i = 0; target && units && units[i]; i++)
		core_action_travel(units[i], target->pos, get_travel_surface);

	free(units);
}
