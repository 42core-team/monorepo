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

bool core_internal_isPosValid(t_pos pos)
{
	return (pos.y < game.grid_size && pos.x < game.grid_size);
}

void core_internal_reset_actions(void)
{
	if (actions.list)
	{
		for (size_t i = 0; i < actions.count; i++)
		{
			if (actions.list[i].type == ACTION_CREATE)
			{
				core_static_freeStringArray(actions.list[i].data.create.components);
				actions.list[i].data.create.components = NULL;
			}
		}
	}

	free(actions.list);
	actions.list = NULL;
	actions.count = 0;
	actions.capacity = 0;
}
void core_internal_reset_debugData(void)
{
	if (debug_data.entries)
	{
		for (unsigned int i = 0; i < debug_data.count; i++)
		{
			if (debug_data.entries[i].path)
			{
				t_debug_path_node *current = debug_data.entries[i].path;
				while (current)
				{
					t_debug_path_node *to_free = current;
					current = current->next;
					free(to_free);
				}
			}

			if (debug_data.entries[i].info) free(debug_data.entries[i].info);
		}
		free(debug_data.entries);
	}

	debug_data.entries = NULL;
	debug_data.count = 0;
	debug_data.capacity = 0;
}

void core_internal_freeStringArray(char **array)
{
	if (!array) return;

	for (size_t i = 0; array[i]; i++)
		free(array[i]);

	free(array);
}

void core_internal_freeObject(t_obj *obj)
{
	if (obj->type == OBJ_UNIT) core_internal_freeStringArray(obj->s_unit.components);
	free(obj);
}
void core_internal_freeObjects(void)
{
	if (game.objects)
	{
		for (int i = 0; game.objects[i]; i++)
		{
			core_internal_freeObject(game.objects[i]);
		}
		free(game.objects);
		game.objects = NULL;
	}
}

void core_internal_freeAndExit(const char *msg, int count, ...)
{
	core_internal_reset_actions();
	core_internal_freeObjects();

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
