#include "core_lib_internal.h"

#include <stdarg.h>

int core_internal_distance(t_pos pos1, t_pos pos2)
{
	double deltaX = (double)pos1.x - (double)pos2.x;
	double deltaY = (double)pos1.y - (double)pos2.y;

	if (deltaX < 0) deltaX = -deltaX;
	if (deltaY < 0) deltaY = -deltaY;

	return ((int)(deltaX + deltaY));
}

void core_internal_resetActions(void)
{
	free(actions.list);
	actions.list = NULL;
	actions.count = 0;
	actions.capacity = 0;
}

void core_internal_freeGame(void)
{
	if (game.objects)
	{
		for (int i = 0; game.objects[i]; i++)
		{
			free(game.objects[i]);
		}
		free(game.objects);
		game.objects = NULL;
	}

	if (game.config.units)
	{
		for (int i = 0; game.config.units[i]; i++)
		{
			free(game.config.units[i]->name);
			free(game.config.units[i]);
		}
		free(game.config.units);
		game.config.units = NULL;
	}
}

void core_internal_freeAndExit(const char *msg, int count, ...)
{
	core_internal_resetActions();
	core_internal_freeGame();

	va_list ap;
	va_start(ap, count);
	for (int i = 0; i < count; ++i)
	{
		void *p = va_arg(ap, void *);
		free(p);
	}
	va_end(ap);

	fprintf(stderr, "FATAL ERROR: %s\n", msg);
	exit(EXIT_FAILURE);
}
