#include "con_lib.h"

static bool is_unit(t_obj *obj)
{
    return (obj->type == OBJ_UNIT && obj->state == STATE_ALIVE);
}
static bool is_unit_own(t_obj *obj)
{
    return (is_unit(obj) && obj->s_unit.team_id == game.my_team_id);
}
static bool is_unit_opponent(t_obj *obj)
{
    return (is_unit(obj) && obj->s_unit.team_id != game.my_team_id);
}

t_obj **core_get_units_own(void)
{
    return core_get_obj_customCondition(is_unit_own);
}
t_obj **core_get_units_opponent(void)
{
    return core_get_obj_customCondition(is_unit_opponent);
}
t_obj *core_get_units_opponent_nearest(t_pos pos)
{
    return core_get_obj_customCondition_nearest(pos, is_unit_opponent);
}
