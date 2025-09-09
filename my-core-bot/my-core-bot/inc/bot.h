#ifndef BOT_H
#define BOT_H

#include "core_lib.h"

t_obj *ft_get_core_own(void);
t_obj *ft_get_core_opponent(void);
t_obj *ft_get_deposit_nearest(t_pos pos);
t_obj *ft_get_units_opponent_nearest(t_pos pos);
t_obj **ft_get_units_own(void);
t_obj **ft_get_units_opponent(void);

void ft_pathfind(t_obj *moving_unit, t_pos target_pos);

#endif /* BOT_H */
