#include "socket.h"
#include "utils.h"
#include "parse_json.h"

int		socket_fd = -1;
bool	debug = false;

void	ft_recieve_config()
{
	char	*msg = ft_read_socket(socket_fd);

	if (debug)
		printf("Recieved: %s\n", msg);
	ft_parse_json_config(msg);
	if (debug)
		ft_print_game_config(&game.config);
	free(msg);
}

void	ft_init_con(int *argc, char **argv)
{
	char	*msg;

	socket_fd = ft_init_socket(ft_init_addr("127.0.0.1", 4242));
	if (*argc == 2)
	{
		msg = malloc(sizeof(char) * (strlen(argv[1]) + 20));
		sprintf(msg, "{\"id\": %s}\n", argv[1]);
	}
	else
		msg = strdup("{\"id\": 1}\n");
	ft_send_socket(socket_fd, msg);
	free(msg);

	ft_recieve_config();
	ft_reset_actions();
}

void	ft_close_con()
{
	close(socket_fd);
}

void	ft_enabled_debug()
{
	debug = true;
}

void	ft_loop(void (*ft_user_loop)())
{
	char	*msg;
	char	*actions;

	while (game.status != STATUS_END)
	{
		actions = ft_all_action_json();
		if (debug)
			printf("Actions: %s\n", actions);
		ft_send_socket(socket_fd, actions);
		free(actions);

		msg = ft_read_socket(socket_fd);
		if (debug)
			printf("Recieved: %s\n", msg);
		ft_parse_json_state(msg);
		free(msg);
		ft_user_loop();
	}
}
