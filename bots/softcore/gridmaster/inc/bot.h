#ifndef BOT_H
#define BOT_H

#include "core_lib.h"

t_obj *ft_get_core_own(void);
t_obj *ft_get_core_opponent(void);
t_obj *ft_get_deposit_nearest(t_pos pos);
t_obj *ft_get_gems_nearest(t_pos pos);
t_obj *ft_get_deposit_gems_nearest(t_pos pos);
t_obj *ft_get_units_opponent_nearest(t_pos pos);
t_obj **ft_get_units_own(void);
t_obj **ft_get_units_opponent(void);

#endif /* BOT_H */
