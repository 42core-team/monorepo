#include "core_lib_internal.h"

int core_internal_util_distance(t_pos pos1, t_pos pos2)
{
	double deltaX = (double)pos1.x - (double)pos2.x;
	double deltaY = (double)pos1.y - (double)pos2.y;

	if (deltaX < 0) deltaX = -deltaX;
	if (deltaY < 0) deltaY = -deltaY;

	return ((int)(deltaX + deltaY));
}

void core_internal_reset_actions(void)
{
	free(actions.list);
	actions.list = NULL;
	actions.count = 0;
	actions.capacity = 0;
}
