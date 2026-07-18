#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("Gridmaster", argc, argv, ft_on_tick, false);
}

static bool is_core_opponent(const t_obj *obj)
{
	return (obj->type == OBJ_CORE && obj->s_core.team_id != game.my_team_id);
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

	core_action_createUnit("Warrior", "combat", "demolition", "health", NULL);

	t_obj *target = core_get_obj_filter_nearest((t_pos){0, 0}, is_core_opponent);
	for (int i = 0; target && game.objects && game.objects[i]; i++)
		if (game.objects[i]->type == OBJ_UNIT && game.objects[i]->s_unit.team_id == game.my_team_id)
			core_action_travel(game.objects[i], target->pos, get_travel_surface);
}
