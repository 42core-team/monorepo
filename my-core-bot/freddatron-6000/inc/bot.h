#ifndef BOT_H
#define BOT_H

#include "core_lib.h"

void move_unit_to(t_obj *unit, t_pos target, bool next_to);

t_obj *ft_get_core_own(void);
t_obj *ft_get_core_opponent(void);
t_obj **ft_get_units_own(void);
t_obj **ft_get_units_opponent(void);

t_obj *ft_get_deposit_nearest(t_obj *unit);
t_obj *ft_get_gempile_nearest(t_obj *unit);
t_obj *ft_get_deposit_gempile_nearest(t_obj *unit);
t_obj *ft_get_units_opponent_nearest(t_obj *unit);

unsigned long ft_path_cost_to(t_obj *unit, t_pos target, int *out_path_len);

#endif /* BOT_H */
