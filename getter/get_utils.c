#include "con_lib.h"

long	ft_distance(t_obj *obj1, t_obj *obj2)
{
	if (!obj1 || !obj2)
		return (__LONG_MAX__);

	long	x = obj1->x - obj2->x;
	long	y = obj1->y - obj2->y;

	return (x * x + y * y);
}
