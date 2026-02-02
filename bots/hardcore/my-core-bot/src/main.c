#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

// NOT SURE HOW TO GET STARTED?
// --> Check out the wiki coregame.sh/wiki/ with a helpful "My first Core bot" beginners guide!

static bool ft_is_own_team_warrior(const t_obj *obj)
{
	if (obj == NULL) return false;
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_WARRIOR) return false;
	if (obj->s_unit.team_id != game.my_team_id) return false;
	return true;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_WARRIOR);

	t_obj **own_team_warriors = core_get_objs_filter(ft_is_own_team_warrior);
	for (int i = 0; own_team_warriors && own_team_warriors[i]; i++)
	{
		t_path path = pathfind_full_path_dijkstra(own_team_warriors[i]->pos, ft_get_core_opponent()->pos);

		// Add all path steps to debug visualization
		for (size_t j = 0; j < path.length; j++)
		{
			core_debug_addObjectPathStep(own_team_warriors[i], path.steps[j]);
		}

		// Move to first step if path exists
		if (path.length > 0)
		{
			core_action_move(own_team_warriors[i], path.steps[0]);
		}

		core_debug_addObjectInfo(own_team_warriors[i], "Moving towards opponent core :D\n");
		core_action_attack(own_team_warriors[i], ft_get_core_opponent());

		// Clean up
		free(path.steps);
	}
	free(own_team_warriors);
}
