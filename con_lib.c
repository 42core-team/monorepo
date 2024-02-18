#include "socket.h"
#include "utils.h"
#include "parse_json.h"

int		socket_fd = -1;
bool	debug = false;
t_game	game;

void	ft_receive_config()
{
	char	*msg = ft_read_socket_once(socket_fd);

	if (debug)
		printf("Received: %s\n", msg);
	ft_parse_json_config(msg);
	if (debug)
		ft_print_game_config(&game.config);
	free(msg);
}

/**
 * @brief Starts the connection to the server. This function should be called before any other function from this library.
 *
 * @param argc The argc from the main function.
 * @param argv The argv from the main function.
 */
void	ft_init_con(int *argc, char **argv)
{
	char	*msg, *env_id;

	socket_fd = ft_init_socket(ft_init_addr("127.0.0.1", 4242));
	env_id = getenv("CORE_ID");
	if (env_id != NULL)
	{
		msg = malloc(sizeof(char) * (strlen(env_id) + 12));
		sprintf(msg, "{\"id\": %s}\n", env_id);
	}
	else if (*argc == 2)
	{
		msg = malloc(sizeof(char) * (strlen(argv[1]) + 12));
		sprintf(msg, "{\"id\": %s}\n", argv[1]);
	}
	else
		msg = strdup("{\"id\": 1}\n");
	ft_send_socket(socket_fd, msg);
	free(msg);

	ft_receive_config();
	ft_reset_actions();
}

/// @brief Closes the connection to the server and frees all allocated memory.
void	ft_close_con()
{
	close(socket_fd);
	ft_free_all();
}

/// @brief Enables debug mode, which prints all sent and received messages.
void	ft_enable_debug()
{
	debug = true;
}

/**
 * @brief Starts the main loop that sends and receives messages from the server. This function should be called after ft_init_con.
 *
 * @param ft_user_loop Your own function that is called every time new data is received.
 */
void	ft_loop(void (*ft_user_loop)())
{
	char	*msg;
	char	*actions;

	while (game.status != STATUS_END)
	{
		actions = ft_all_action_json();
		ft_reset_actions();
		if (debug)
			printf("Actions: %s\n", actions);
		ft_send_socket(socket_fd, actions);
		free(actions);

		msg = ft_read_socket(socket_fd);
		if (debug)
			printf("Received: %s\n", msg);
		ft_parse_json_state(msg);
		free(msg);
		ft_user_loop();
	}
}
