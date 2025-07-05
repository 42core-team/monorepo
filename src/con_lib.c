#include "socket.h"
#include "parse_json.h"

t_game	game = {0};

/**
 * @brief Starts the connection to the server. This function should be called before any other function from this library.
 *
 * @param team_name The name of your team.
 * @param argc The argc from the main function.
 * @param argv The argv from the main function.
 * @param tick_callback A function that will be called every game tick.
 * @param debug Whether to enable debug mode or not.
 */
int	ft_game_start(char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug)
{
	// clean structs
	memset(&game, 0, sizeof(t_game));
	memset(&game.config, 0, sizeof(t_config));
	memset(&game.actions, 0, sizeof(t_actions));
	ft_reset_actions();

	// setup socket
	const char *env_ip = getenv("SERVER_IP");
	const char *env_port = getenv("SERVER_PORT");
	const int port = env_port ? atoi(env_port) : 4242;
	game.my_team_id = argv[1] ? atoi(argv[1]) : 0;
	int socket_fd = ft_init_socket(ft_init_addr(env_ip ? env_ip : "127.0.0.1", port));

	// send login message
	char *login_msg = ft_create_login_msg(team_name, argc, argv);
	if (!login_msg)
	{
		printf("Unable to create login message, shutting down.\n");
		return (1);
	}
	ft_send_socket(socket_fd, login_msg);
	free(login_msg);

	// receive config
	char *conf = ft_read_socket_once(socket_fd);
	if (!conf)
	{
		printf("Something went very awry and there was no json received.\n");
		return 1;
	}
	if (debug)
		printf("Received: %s\n", conf);
	ft_parse_json_config(conf);
	free(conf);

	
	// run game loop
	printf("Game started! Your id: %ld\n", game.my_team_id);
	bool first_tick = true;
	while ((ft_get_my_core() != NULL && ft_get_my_core()->hp > 0) || first_tick)
	{
		first_tick = false;

		// send user-selected actions
		char *actions = ft_all_action_json();
		ft_reset_actions();
		if (debug)
			printf("Actions: %s\n", actions);
		ft_send_socket(socket_fd, actions);
		free(actions);

		// receive new json state
		char *msg = ft_read_socket(socket_fd);
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

		// execute user code
		if (tick_callback)
			tick_callback(game.elapsed_ticks);
	}

	// handle game end
	if (ft_get_my_core() && ft_get_my_core()->hp > 0)
		printf("Game over! You won!\n");
	else
		printf("Game over! You lost!\n");

	// clean up
	close(socket_fd);
	ft_free_game();
}
