#include "parse_json.h"

void	ft_free_game()
{
	if (game.cores != NULL)
	{
		for (int i = 0; game.cores[i] != NULL; i++)
			free(game.cores[i]);
		free(game.cores);
		game.cores = NULL;
	}
	if (game.resources != NULL)
	{
		for (int i = 0; game.resources[i] != NULL; i++)
			free(game.resources[i]);
		free(game.resources);
		game.resources = NULL;
	}
	if (game.units != NULL)
	{
		for (int i = 0; game.units[i] != NULL; i++)
			free(game.units[i]);
		free(game.units);
		game.units = NULL;
	}
	if (game.walls != NULL)
	{
		for (int i = 0; game.walls[i] != NULL; i++)
			free(game.walls[i]);
		free(game.walls);
		game.walls = NULL;
	}
}

void	ft_free_config()
{
	int	ind;

	ind = 0;
	if (game.config.units != NULL)
	{
		while (game.config.units[ind] != NULL)
		{
			free(game.config.units[ind]->name);
			free(game.config.units[ind]);
			ind++;
		}
		free(game.config.units);
	}
}

void	ft_free_all()
{
	ft_free_game();
	ft_free_config();
	ft_reset_actions();
}

void	ft_perror_exit(char *msg)
{
	perror(msg);
	ft_free_all();
	exit(EXIT_FAILURE);
}
