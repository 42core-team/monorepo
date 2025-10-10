#include "bot.h"

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("YOUR TEAM NAME HERE", argc, argv, ft_on_tick, false);
}

static bool ft_is_own_team_miner(const t_obj *obj)
{
	if (obj == NULL) return false;
	if (obj->type != OBJ_UNIT) return false;
	if (obj->s_unit.unit_type != UNIT_MINER) return false;
	if (obj->s_unit.team_id != game.my_team_id) return false;
	if (obj->state != STATE_ALIVE) return false;
	return true;
}

void ft_on_tick(unsigned long tick)
{
	printf("-----> [⚡️ TICK %ld 🔥]\n", tick);

	core_action_createUnit(UNIT_MINER);

	t_obj **own_team_miners = core_get_objs_filter(ft_is_own_team_miner);
	for (int i = 0; own_team_miners && own_team_miners[i]; i++)
	{
		if (own_team_miners[i]->s_unit.gems > 0)
		{
			core_action_moveTowards(own_team_miners[i], ft_get_core_own()->pos);
			core_action_transferGems_toObj(own_team_miners[i], ft_get_core_own(), own_team_miners[i]->s_unit.gems);
		}
		else
		{
			t_obj *nearest_gems = ft_get_deposit_gems_nearest(own_team_miners[i]->pos);
			core_action_moveTowards(own_team_miners[i], nearest_gems->pos);
			core_action_attack_object(own_team_miners[i], nearest_gems); // Attack is used to mine gems
		}
	}
	free(own_team_miners);
}
