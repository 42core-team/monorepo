#include "con_lib.h"

double	ft_distance(t_obj *obj1, t_obj *obj2)
{
	if (!obj1 || !obj2)
		return (__DBL_MAX__);

	double	x = (double) obj1->x - (double) obj2->x;
	double	y = (double) obj1->y - (double) obj2->y;

	return (sqrt(x * x + y * y));
}
