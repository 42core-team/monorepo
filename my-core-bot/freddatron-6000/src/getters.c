#include "bot.h"

static t_obj *pick_cheapest_for_unit(t_obj **candidates, t_obj *unit, unsigned long *out_cost)
{
	if (!candidates) return NULL;
	t_obj *best = NULL;
	unsigned long best_cost = ULONG_MAX;
	for (int i = 0; candidates[i]; ++i)
	{
		if (candidates[i]->state != STATE_ALIVE) continue;
		unsigned long c = ft_path_cost_to(unit, candidates[i]->pos, NULL);
		if (c < best_cost)
		{
			best_cost = c;
			best = candidates[i];
		}
	}
	if (out_cost) *out_cost = best_cost;
	if (best_cost == ULONG_MAX) return NULL;
	free(candidates);
	return best;
}


static bool is_core(const t_obj *obj)
{
	return (obj->type == OBJ_CORE && obj->state == STATE_ALIVE);
}
static bool is_core_own(const t_obj *obj)
{
	return (is_core(obj) && obj->s_core.team_id == game.my_team_id);
}
static bool is_core_opponent(const t_obj *obj)
{
	return (is_core(obj) && obj->s_core.team_id != game.my_team_id);
}

static bool is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT && obj->state == STATE_ALIVE);
}
static bool is_gem_pile(const t_obj *obj)
{
	return (obj->type == OBJ_GEM_PILE && obj->state == STATE_ALIVE);
}
static bool is_resource_money(const t_obj *obj)
{
	return (is_deposit(obj) || is_gem_pile(obj));
}

static bool is_unit(const t_obj *obj)
{
	return (obj->type == OBJ_UNIT && obj->state == STATE_ALIVE);
}
static bool is_unit_own(const t_obj *obj)
{
	return (is_unit(obj) && obj->s_unit.team_id == game.my_team_id);
}
static bool is_unit_opponent(const t_obj *obj)
{
	return (is_unit(obj) && obj->s_unit.team_id != game.my_team_id);
}

// -

t_obj *ft_get_core_own(void)
{
	t_pos pos = {0, 0};
	return core_get_obj_filter_nearest(pos, is_core_own);
}
t_obj *ft_get_core_opponent(void)
{
	t_pos pos = {0, 0}; // Position doesn't matter for this search
	return core_get_obj_filter_nearest(pos, is_core_opponent);
}

t_obj *ft_get_deposit_nearest(t_obj *unit)
{
	t_obj **all = core_get_objs_filter(is_deposit);
	return pick_cheapest_for_unit(all, unit, NULL);
}
t_obj *ft_get_gempile_nearest(t_obj *unit)
{
	t_obj **all = core_get_objs_filter(is_gem_pile);
	return pick_cheapest_for_unit(all, unit, NULL);
}
t_obj *ft_get_deposit_gempile_nearest(t_obj *unit)
{
	t_obj **all = core_get_objs_filter(is_resource_money);
	return pick_cheapest_for_unit(all, unit, NULL);
}

t_obj *ft_get_units_opponent_nearest(t_obj *unit)
{
	t_obj **all = core_get_objs_filter(is_unit_opponent);
	return pick_cheapest_for_unit(all, unit, NULL);
}
t_obj **ft_get_units_own(void)
{
	return core_get_objs_filter(is_unit_own);
}
t_obj **ft_get_units_opponent(void)
{
	return core_get_objs_filter(is_unit_opponent);
}
