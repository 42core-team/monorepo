#include "bot.h"

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

static bool is_resource(const t_obj *obj)
{
	return (obj->type == OBJ_RESOURCE && obj->state == STATE_ALIVE);
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
	t_pos pos = {0, 0}; // Position doesn't matter for this search
	return core_get_obj_customCondition_nearest(pos, is_core_own);
}
t_obj *ft_get_core_opponent(void)
{
	t_pos pos = {0, 0}; // Position doesn't matter for this search
	return core_get_obj_customCondition_nearest(pos, is_core_opponent);
}

t_obj *ft_get_resource_nearest(t_pos pos)
{
	return core_get_obj_customCondition_nearest(pos, is_resource);
}

t_obj **ft_get_units_own(void)
{
	return core_get_objs_customCondition(is_unit_own);
}
t_obj **ft_get_units_opponent(void)
{
	return core_get_objs_customCondition(is_unit_opponent);
}
t_obj *ft_get_units_opponent_nearest(t_pos pos)
{
	return core_get_obj_customCondition_nearest(pos, is_unit_opponent);
}
