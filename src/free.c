#include "core_lib_internal.h"

void	ft_free_game()
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

	ft_reset_actions();
}

void	ft_perror_exit(char *msg)
{
	perror(msg);
	ft_free_game();
	exit(EXIT_FAILURE);
}
