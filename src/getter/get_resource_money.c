#include "con_lib.h"

static bool is_resource(t_obj *obj)
{
    return (obj->type == OBJ_RESOURCE && obj->state == STATE_ALIVE);
}
static bool is_money(t_obj *obj)
{
    return (obj->type == OBJ_MONEY && obj->state == STATE_ALIVE);
}
static bool is_resource_or_money(t_obj *obj)
{
    return (is_resource(obj) || is_money(obj));
}

t_obj *ft_get_resource_nearest(t_pos pos)
{
    return core_get_obj_nearest_custom_condition(pos, is_resource);
}
t_obj *ft_get_money_nearest(t_pos pos)
{
    return core_get_obj_nearest_custom_condition(pos, is_money);
}
t_obj *ft_get_resource_money_nearest(t_pos pos)
{
    return core_get_obj_nearest_custom_condition(pos, is_resource_or_money);
}
