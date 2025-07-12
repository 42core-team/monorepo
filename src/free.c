#include "core_lib_internal.h"

void	core_internal_util_freeGame()
{
	// Objects
	if (game.objects != NULL)
	{
		for (int i = 0; game.objects[i] != NULL; i++)
			free(game.objects[i]);
		free(game.objects);
		game.objects = NULL;
	}

	// Config
	if (game.config.units != NULL)
	{
		for (int i = 0; game.config.units[i] != NULL; i++)
		{
			free(game.config.units[i]->name);
			free(game.config.units[i]);
		}
		free(game.config.units);
	}

	core_internal_reset_actions();
}
