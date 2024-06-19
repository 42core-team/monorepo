#include "con_lib.h"

int	ft_distance(t_obj *obj1, t_obj *obj2)
{
	int	x = obj1->x - obj2->x;
	int	y = obj1->y - obj2->y;

	return (x * x + y * y);
}
