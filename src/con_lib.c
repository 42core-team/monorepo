#include "socket.h"
#include "parse_json.h"
#include "event_handler.h"

int		socket_fd = -1;
bool	debug = false;
t_game	game = {0};
t_event_handler event_handler = {0};
void	*user_data = NULL;

bool	ft_receive_config()
{
	char	*msg = ft_read_socket_once(socket_fd);

	if (!msg)
	{
		printf("Something went very awry and there was no json received.\n");
		return false;
	}
	if (debug)
		printf("Received: %s\n", msg);
	ft_parse_json_config(msg);
	free(msg);
	return true;
}

/**
 * @brief Starts the connection to the server. This function should be called before any other function from this library.
 *
 * @param team_name The name of your team.
 * @param argc The argc from the main function.
 * @param argv The argv from the main function.
 */
void	ft_init_con(char *team_name, int argc, char **argv)
{
	memset(&game, 0, sizeof(t_game));
	memset(&game.config, 0, sizeof(t_config));
	memset(&game.actions, 0, sizeof(t_actions));

	const char *env_ip = getenv("SERVER_IP");
	const char *env_port = getenv("SERVER_PORT");
	const int port = env_port ? atoi(env_port) : 4242;
	game.my_team_id = argv[1] ? atoi(argv[1]) : 0;

	socket_fd = ft_init_socket(ft_init_addr(env_ip ? env_ip : "127.0.0.1", port));

	char *login_msg = ft_create_login_msg(team_name, argc, argv);
	if (!login_msg)
	{
		printf("Unable to create login message, shutting down.\n");
		exit(1);
	}
	ft_send_socket(socket_fd, login_msg);
	free(login_msg);

	ft_reset_actions();
	bool validConf = ft_receive_config();

	if (!validConf)
	{
		printf("Unable to initialize server connection, shutting down.\n");
		exit(1);
	}
	printf("Game started! Your id: %ld\n", game.my_team_id);
}

/// @brief Closes the connection to the server and frees all allocated memory.
void	ft_close_con()
{
	close(socket_fd);
	ft_free_game();
}

/// @brief Enables debug mode, which prints all sent and received messages.
void	ft_enable_debug()
{
	debug = true;
}

/**
 * @brief Starts the main loop that sends and receives messages from the server. This function should be called after ft_init_con.
 *
 * @param ft_init_func Your own function that is called once at the start of the game.
 * @param ft_user_loop Your own function that is called every time new data is received.
 * @param ptr A pointer that is passed to your functions.
 */
void ft_loop(t_event_handler handler, void *custom_data)
{
	char	*msg;
	char	*actions;

	event_handler = handler;
	user_data = custom_data;
	if (event_handler.on_start)
		event_handler.on_start(custom_data);

	bool first_tick = true;
	while ((ft_get_my_core() != NULL && ft_get_my_core()->hp > 0) || first_tick)
	{
		first_tick = false;

		actions = ft_all_action_json();
		ft_reset_actions();
		if (debug)
			printf("Actions: %s\n", actions);
		ft_send_socket(socket_fd, actions);
		free(actions);

		msg = ft_read_socket(socket_fd);
		if (!msg)
		{
			printf("The connection was closed by the server. Bye, bye!\n");
			break;
		}
		if (debug)
		{
			json_node *node = string_to_json(msg);
			printf("Received: %s\n", json_to_formatted_string(node));
			free_json(node);
		}

		ft_parse_json_state(msg);
		free(msg);

		if (event_handler.on_tick)
			event_handler.on_tick(game.elapsed_ticks, custom_data);
		if (event_handler.on_object_ticked)
			for (size_t i = 0; game.objects && game.objects[i]; i++)
				event_handler.on_object_ticked(game.objects[i], game.elapsed_ticks, custom_data);
	}

	if (event_handler.on_exit)
		event_handler.on_exit(custom_data);

	if (ft_get_my_core() && ft_get_my_core()->hp > 0)
		printf("Game over! You won!\n");
	else
		printf("Game over! You lost!\n");
}
