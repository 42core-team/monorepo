#include "core_lib_internal.h"

#include <string.h>
#include <sys/select.h>

t_game game = {0};
t_actions actions = {0};

static void core_static_awaitEnterPress(void)
{
	if (!isatty(STDIN_FILENO)) return;
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

int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug)
{
	(void)argc;

	if (!tick_callback)
	{
		printf("Trust me, you'll want to provide a tick callback function.\n");
		return 1;
	}

	// Parse team id from args
	if (!argv[1]) return printf("Error: No team id provided.\n"), 1;
	game.my_team_id = atoi(argv[1]);

	// Resolve server address
	const char *env_ip = getenv("SERVER_IP");
	const char *env_port = getenv("SERVER_PORT");
	const int port = env_port ? atoi(env_port) : 4444;
	const char *host = env_ip ? env_ip : "127.0.0.1";

	// Connect via gRPC
	printf("Connecting to server at %s:%d\n", host, port);
	if (grpc_bridge_connect(host, port) != 0)
	{
		fprintf(stderr, "Failed to connect to server\n");
		return 1;
	}

	// Login
	if (grpc_bridge_login(game.my_team_id, "42", team_name ? team_name : "Unnamed") != 0)
	{
		fprintf(stderr, "Login failed\n");
		return 1;
	}
	if (debug) printf("Logged in as team %lu\n", game.my_team_id);

	// Open bidirectional tick stream
	if (grpc_bridge_start_tick_stream() != 0)
	{
		fprintf(stderr, "Failed to start tick stream\n");
		return 1;
	}

	// Game loop
	bool won = false;
	while (1)
	{
		unsigned long tick = 0;
		char **errors = NULL;
		int error_count = 0;
		bool game_over = false;
		unsigned long winner_team_id = 0;

		// Clear per-tick cache from previous tick
		grpc_bridge_cache_clear();

		// Wait for next tick signal from server
		if (grpc_bridge_wait_tick(&tick, &errors, &error_count, &game_over, &winner_team_id) != 0)
		{
			printf("The connection was closed by the server. Bye, bye!\n");
			break;
		}

		game.elapsed_ticks = tick;

		// Print errors from last tick
		for (int i = 0; i < error_count; i++)
		{
			printf("\033[31m%s\033[0m\n", errors[i]);
			free(errors[i]);
		}
		free(errors);

		if (game_over)
		{
			won = (winner_team_id == game.my_team_id);
			break;
		}

		if (debug) printf("Tick %lu\n", tick);

		// Execute user code
		tick_callback(game.elapsed_ticks);

		// Send accumulated debug data
		grpc_bridge_send_debug_data();
		core_internal_reset_debugData();

		// Signal end of turn
		grpc_bridge_end_turn();
	}

	// Handle game end
	core_static_awaitEnterPress();
	if (won)
		printf("Game over! You won!\n");
	else
		printf("Game over! You lost!\n");

	// Clean up
	grpc_bridge_cache_clear();
	grpc_bridge_shutdown();
	core_internal_freeGame();
	core_internal_reset_actions();

	return 0;
}
