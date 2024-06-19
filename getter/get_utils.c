#include "con_lib.h"

double	ft_distance(t_obj *obj1, t_obj *obj2)
{
	double	x = obj1->x - obj2->x;
	double	y = obj1->y - obj2->y;

	return (x * x + y * y);
}
