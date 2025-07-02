#include "con_lib.h"

int core_util_distance(t_pos pos1, t_pos pos2)
{
	double x = (double)pos1.x - (double)pos2.x;
	double y = (double)pos1.y - (double)pos2.y;

	if (x < 0)
		x = -x;
	if (y < 0)
		y = -y;

	return ((int)(x + y));
}
