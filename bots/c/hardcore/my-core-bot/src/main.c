#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, true);
}

// NOT SURE HOW TO GET STARTED?
// --> Check out the wiki coregame.sh/wiki/ with a helpful "My first Core bot" beginners guide!

static bool is_unit_own(const t_obj *obj)
{
	return (obj->type == OBJ_UNIT && obj->s_unit.team_id == game.my_team_id);
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(NULL, "unit_attack", "core_attack", "health", NULL);

	t_obj **own_team_units = core_get_objs_filter(is_unit_own);
	for (int i = 0; own_team_units && own_team_units[i]; i++)
	{
		t_obj *target = ft_get_units_opponent_nearest(own_team_units[i]->pos);
		if (!target) target = ft_get_core_opponent();

		t_path path = pathfind_full_path_dijkstra(own_team_units[i]->pos, target->pos);

		// Add all path steps to debug visualization
		for (size_t j = 0; j < path.length; j++)
		{
			core_debug_addObjectPathStep(own_team_units[i], path.steps[j]);
		}

		// Move to first step if path exists
		if (path.length > 0)
		{
			core_action_move(own_team_units[i], path.steps[0]);
		}

		core_debug_addObjectInfo(own_team_units[i], "Moving towards target at [%d,%d] :D\n", target->pos.x,
								 target->pos.y);
		core_action_attack(own_team_units[i], target);

		// Clean up
		free(path.steps);
	}
	free(own_team_units);
}
