#include "bot.h"

#include <stdio.h>
#include <time.h>

void ft_on_tick(unsigned long tick);
void ft_on_object_ticked(t_obj *unit, unsigned long tick);
int max(int a, int b);

int main(int argc, char **argv)
{
	return core_startGame("FREDDATRON 6000 👾🤖💥", argc, argv, ft_on_tick, false);
}

int ft_util_actionCooldown(t_obj *unit)
{
	t_unit_config *uconf = core_get_unitConfig(unit->s_unit.unit_type);
	unsigned int gems = max(unit->s_unit.gems, 1);
	unsigned int ac = uconf->baseActionCooldown + gems / uconf->balancePerCooldownStep;
	if (ac > uconf->maxActionCooldown) ac = uconf->maxActionCooldown;
	if (ac < 1) ac = 1;
	return (ac);
}

int ft_util_distance(t_pos pos1, t_pos pos2)
{
	double x = (double)pos1.x - (double)pos2.x;
	double y = (double)pos1.y - (double)pos2.y;

	if (x < 0) x = -x;
	if (y < 0) y = -y;

	return ((int)(x + y));
}

int nextUnit = 2;

void ft_on_tick(unsigned long tick)
{
	(void)tick;

	printf("tick %lu\n", tick);

	if (ft_get_core_own() && ft_get_core_own()->s_core.gems >= core_get_unitConfig(nextUnit)->cost)
	{
		core_action_createUnit(nextUnit);
		nextUnit--;
		if (nextUnit < 0) nextUnit = 2;
	}

	for (int i = 0; game.objects && game.objects[i]; i++)
		ft_on_object_ticked(game.objects[i], tick);
}

void ft_on_object_ticked(t_obj *unit, unsigned long tick)
{
	(void)tick;

	if (unit->state != STATE_ALIVE) return;
	if (unit->type != OBJ_UNIT) return;
	if (unit->s_unit.team_id != ft_get_core_own()->s_core.team_id) return;

	if (unit->s_unit.gems > 0 && unit->s_unit.unit_type != UNIT_CARRIER)
	{
		core_action_transferGems(unit, (t_pos){unit->pos.x + 1, unit->pos.y}, unit->s_unit.gems);
		return;
	}

	int typeId = unit->s_unit.unit_type;
	if (typeId == UNIT_WARRIOR)
	{
		t_obj *nearestOpponent = ft_get_units_opponent_nearest(unit);
		if (nearestOpponent)
			move_unit_to(unit, nearestOpponent->pos, false);
		else
			move_unit_to(unit, ft_get_core_opponent()->pos, false);
	}
	else if (typeId == UNIT_MINER)
	{
		t_obj *nearestResource = ft_get_deposit_nearest(unit);
		if (nearestResource) move_unit_to(unit, nearestResource->pos, false);
	}
	else if (typeId == UNIT_CARRIER)
	{
		unsigned int balance = unit->s_unit.gems;
		t_obj *nearestMoney = ft_get_gempile_nearest(unit);
		if (balance > 250 || !nearestMoney)
		{
			move_unit_to(unit, ft_get_core_own()->pos, true);
			if (balance > 0 && ft_util_distance(unit->pos, ft_get_core_own()->pos) <= 1)
				core_action_transferGems(unit, ft_get_core_own()->pos, balance);
		}
		else
		{
			if (nearestMoney) move_unit_to(unit, nearestMoney->pos, false);
			// else
			// 	move_unit_to(unit, ft_get_core_opponent()->pos);
		}
	}
}
