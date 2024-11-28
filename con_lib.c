#include "socket.h"
#include "utils.h"
#include "parse_json.h"

int		socket_fd = -1;
bool	debug = false;
t_game	game;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* json_formatter(const char* json_input)
{
	int len = strlen(json_input);
	int max_output_len = len * 2; // Estimate output size
	char *output = (char*)malloc(max_output_len);
	if (output == NULL) return NULL;

	int indent_level = 0;
	int in_string = 0;
	int i, j = 0;

	for (i = 0; i < len; i++) {
		char ch = json_input[i];

		if (ch == '\"') {
			// Check for escaped quotes
			int escape = 0;
			int k = i - 1;
			while (k >= 0 && json_input[k] == '\\') {
				escape = !escape;
				k--;
			}
			if (!escape) in_string = !in_string;
			output[j++] = ch;
		} else if (!in_string) {
			if (ch == '{' || ch == '[') {
				output[j++] = ch;
				output[j++] = '\n';
				indent_level++;
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
			} else if (ch == '}' || ch == ']') {
				output[j++] = '\n';
				indent_level--;
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
				output[j++] = ch;
			} else if (ch == ',') {
				output[j++] = ch;
				output[j++] = '\n';
				for (int l = 0; l < indent_level; l++) output[j++] = '\t';
			} else if (ch == ':') {
				output[j++] = ch;
				output[j++] = ' ';
			} else if (ch != ' ' && ch != '\n' && ch != '\t') {
				output[j++] = ch;
			}
		} else {
			output[j++] = ch;
		}

		// Resize output buffer if needed
		if (j >= max_output_len - 1) {
			max_output_len *= 2;
			char* temp = (char*)realloc(output, max_output_len);
			if (temp == NULL) {
				free(output);
				return NULL;
			}
			output = temp;
		}
	}
	output[j] = '\0';
	return output;
}


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

void	ft_init_game()
{
	game.teams = NULL;
	game.cores = NULL;
	game.resources = NULL;
	game.units = NULL;
	game.config.teams = NULL;
	game.config.units = NULL;
	game.config.resources = NULL;
}

/**
 * @brief Starts the connection to the server. This function should be called before any other function from this library.
 *
 * @param team_name The name of your team.
 * @param argc The argc from the main function.
 * @param argv The argv from the main function.
 */
void	ft_init_con(char *team_name, int *argc, char **argv)
{
	const char *env_ip = getenv("SERVER_IP");
	const char *env_port = getenv("SERVER_PORT");
	const int port = env_port ? atoi(env_port) : 4242;

	socket_fd = ft_init_socket(ft_init_addr(env_ip ? env_ip : "127.0.0.1", port));

	char *login_msg = ft_create_login_msg(team_name, argc, argv);
	ft_send_socket(socket_fd, login_msg);
	free(login_msg);

	ft_init_game();
	ft_receive_config();
	ft_reset_actions();

	printf("Game started! Your id: %ld\n", game.my_team_id);
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
 * @param ft_init_func Your own function that is called once at the start of the game.
 * @param ft_user_loop Your own function that is called every time new data is received.
 * @param ptr A pointer that is passed to your functions.
 */
void	ft_loop(void (*ft_init_func)(void *ptr), void (*ft_user_loop)(void *ptr), void *ptr)
{
	char	*msg;
	char	*actions;

	if (ft_init_func != NULL)
		ft_init_func(ptr);
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

		FILE *fp = fopen("data.json", "w");
		char *formatted = json_formatter(msg);
		fprintf(fp, "%s", formatted);
		fclose(fp);
		free(formatted);

		ft_parse_json_state(msg);
		free(msg);
		if (game.status == STATUS_END)
			break;

		ft_user_loop(ptr);
	}

	if (ft_get_my_core())
		printf("Game over! You won!\n");
	else
		printf("Game over! You lost!\n");
}
