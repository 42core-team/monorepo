#include "bot.h"

#include <stdio.h>
#include <time.h>

void ft_on_tick(unsigned long tick);

int main(int argc, char **argv)
{
	return core_startGame("Gridmaster", argc, argv, ft_on_tick, false);
}

int target_unit = 1;

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	// spawn new unit
	if (ft_get_core_own() && ft_get_core_own()->s_core.gems >= core_get_unitConfig(target_unit)->cost)
	{
		core_action_createUnit(target_unit);
		target_unit++;
		if (target_unit > 1) target_unit = 0;
	}

	// move units
	t_obj **units = ft_get_units_own();
	for (int i = 0; units && units[i]; i++)
	{
		t_obj *obj = units[i];
		if (obj->state != STATE_ALIVE) continue;

		switch ((int)obj->s_unit.unit_type)
		{
		case UNIT_WARRIOR:
			t_obj *closest_opponent = ft_get_units_opponent_nearest(ft_get_core_own()->pos);
			if (closest_opponent)
				core_action_moveTowards(obj, closest_opponent->pos);
			else
				core_action_moveTowards(obj, ft_get_core_opponent()->pos);
			break;

		case UNIT_MINER:
			t_obj *nearest_deposit = ft_get_deposit_nearest(obj->pos);
			if (nearest_deposit && obj->s_unit.gems <= 0)
				core_action_moveTowards(obj, nearest_deposit->pos);
			else
			{
				core_action_moveTowards(obj, ft_get_core_own()->pos);
				core_action_transferGems(obj, ft_get_core_own()->pos, -1);
			}
			break;
		}
	}
	free(units);
}
