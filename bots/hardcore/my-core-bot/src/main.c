#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

static bool ft_is_own_team_warrior(const t_obj *obj)
{
	if (obj == NULL) return false;
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_WARRIOR) return false;
	if (obj->s_unit.team_id != game.my_team_id) return false;
	if (obj->state != STATE_ALIVE) return false;
	return true;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **own_team_warriors = core_get_objs_filter(ft_is_own_team_warrior);
	for (int i = 0; own_team_warriors && own_team_warriors[i]; i++)
	{
		core_action_move(own_team_warriors[i],
						 pathfind_next_step_dijkstra(own_team_warriors[i]->pos, ft_get_core_opponent()->pos));
		core_action_attack_obj(own_team_warriors[i], ft_get_core_opponent());
	}
	free(own_team_warriors);
}
