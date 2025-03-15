#include "con_lib.h"

double	ft_distance(t_obj *obj1, t_obj *obj2)
{
	if (!obj1 || !obj2)
		return (__DBL_MAX__);

	double	x = (double) obj1->pos.x - (double) obj2->pos.x;
	double	y = (double) obj1->pos.y - (double) obj2->pos.y;

	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;

	return (x + y);
}
