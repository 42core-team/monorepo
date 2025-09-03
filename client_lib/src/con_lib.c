#include "core_lib_internal.h"

t_game game = {0};
t_actions actions = {0};

static bool core_static_isMyCore(const t_obj *obj)
{
	if (!obj || obj->type != OBJ_CORE)
		return false;
	return (obj->s_core.team_id == game.my_team_id);
}




















static void core_static_awaitEnterPress(void)
{
	if (!isatty(STDIN_FILENO))
		return;
	printf("The game has ended! But who won?...\n");
	printf("Press ENTER to reveal the result...\n");
	fflush(stdout);
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(STDIN_FILENO, &rfds);
	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	int rv = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
	if (rv > 0)
	{
		int c;
		while ((c = getchar()) != '\n' && c != EOF)
		{
		}
	}
}

/**
 * @brief Starts the connection to the server. This function should be called before any other function from this library.
 *
 * @param team_name The name of your team.
 * @param argc The argc from the main function.
 * @param argv The argv from the main function.
 * @param tick_callback A function that will be called every game tick.
 * @param debug Whether to enable debug mode or not.
 */
int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug)
{
	if (!tick_callback)
	{
		printf("Trust me, you'll want to provide a tick callback function.\n");
		return 1;
	}

	// setup socket
	const char *env_ip = getenv("SERVER_IP");
	const char *env_port = getenv("SERVER_PORT");
	const int port = env_port ? atoi(env_port) : 4444;
	if (argv[1])
		game.my_team_id = atoi(argv[1]);
	else
		return printf("Error: No team id provided.\n"), 1;
	struct sockaddr_in server_addr = core_internal_socket_initAddr(env_ip ? env_ip : "127.0.0.1", port);
	int socket_fd = core_internal_socket_init(server_addr);

	// send login message
	char *login_msg = core_internal_encode_login(team_name, argc, argv);
	if (!login_msg)
	{
		printf("Unable to create login message, shutting down.\n");
		return (1);
	}
	core_internal_socket_send(socket_fd, login_msg);
	free(login_msg);

	// receive config
	char *conf = core_internal_socket_read_once(socket_fd);
	if (!conf)
	{
		printf("Something went very awry and there was no json received.\n");
		return 1;
	}
	if (debug)
		printf("Received: %s\n", conf);
	core_internal_parse_config(conf);
	free(conf);

	// run game loop
	bool first_tick = true;
	t_obj *my_core = NULL;
	while ((my_core != NULL && my_core->hp > 0) || first_tick)
	{
		first_tick = false;

		// send user-selected actions
		char *tick_actions = core_internal_encode_action();
		core_internal_reset_actions();
		if (debug)
			printf("Actions: %s\n", tick_actions);
		core_internal_socket_send(socket_fd, tick_actions);
		free(tick_actions);

		// receive new json state
		char *msg = core_internal_socket_read(socket_fd);
		if (!msg)
		{
			printf("The connection was closed by the server. Bye, bye!\n");
			break;
		}
		if (debug)
		{
			json_node *node = string_to_json(msg);
			char *formatted = json_to_formatted_string(node);
			printf("Received: %s\n", formatted);
			free(formatted);
			free_json(node);
		}
		core_internal_parse_state(msg);
		free(msg);
		my_core = core_get_obj_filter_first(core_static_isMyCore);

		// execute user code
		tick_callback(game.elapsed_ticks);
	}

	// handle game end
	core_static_awaitEnterPress();
	if (my_core && my_core->hp > 0)
		printf("Game over! You won!\n");
	else
		printf("Game over! You lost!\n");

	// clean up
	close(socket_fd);
	if (game.objects != NULL)
	{
		for (int i = 0; game.objects[i] != NULL; i++)
			free(game.objects[i]);
		free(game.objects);
		game.objects = NULL;
	}
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

	return 0;
}
