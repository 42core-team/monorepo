#include "con_lib.h"

static bool is_core(t_obj *obj)
{
    return (obj->type == OBJ_CORE && obj->state == STATE_ALIVE);
}
static bool is_core_own(t_obj *obj)
{
    return (is_core(obj) && obj->s_core.team_id == game.my_team_id);
}
static bool is_core_opponent(t_obj *obj)
{
    return (is_core(obj) && obj->s_core.team_id != game.my_team_id);
}

t_obj *core_get_core_own(void)
{
    t_pos pos = {0, 0}; // Position doesn't matter for this search
    return core_get_obj_nearest_custom_condition(pos, is_core_own);
}
t_obj *core_get_core_opponent(void)
{
    t_pos pos = {0, 0}; // Position doesn't matter for this search
    return core_get_obj_nearest_custom_condition(pos, is_core_opponent);
}
