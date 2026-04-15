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

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	core_action_createUnit(UNIT_PAPER_WARRIOR);

	for (int i = 0; game.objects && game.objects[i]; i++)
		if (game.objects[i]->type == OBJ_UNIT && game.objects[i]->s_unit.team_id == game.my_team_id)
			core_action_pathfind(game.objects[i], core_get_obj_filter_nearest((t_pos){0, 0}, is_core_opponent)->pos);
}
