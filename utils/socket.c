#include "socket.h"
#include "utils.h"

int	ft_init_socket(struct sockaddr_in addr)
{
	int	socket_fd, status_con;

	write(1, "Connecting to server", 21);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		ft_perror_exit("Socket creation failed");

	// Set socket to non-blocking mode
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0)
		ft_perror_exit("Get socket flags failed");

	flags |= O_NONBLOCK;
	if (fcntl(socket_fd, F_SETFL, flags) < 0)
		ft_perror_exit("Set socket to non-blocking mode failed");

	do {
		write(1, ".", 1);
		status_con = connect(socket_fd, (struct sockaddr *) &addr, sizeof(addr));
		if (status_con < 0) {
			fd_set waitset;
			FD_ZERO(&waitset);
			FD_SET(socket_fd, &waitset);

			struct timeval timeout;
			timeout.tv_sec = 5;
			timeout.tv_usec = 0;

			status_con = select(socket_fd + 1, NULL, &waitset, NULL, &timeout);
		}
		if (status_con != 0)
			usleep(350000);
	} while (status_con != 0);

	write(1, "Connected!\n", 12);
	return (socket_fd);
}

int	ft_send_socket(const int socket_fd, const char *msg)
{
	if (!msg)
		return (-1);

	int status_send = send(socket_fd, msg, strlen(msg), 0);
	if (status_send < 0)
		ft_perror_exit("Socket send failed");

	return (status_send);
}

char	*ft_read_socket(const int socket_fd)
{
	char	*buffer = NULL;
	char	*last_buffer = NULL;

	while ((buffer = get_next_line(socket_fd)) == NULL);

	while (buffer != NULL)
	{
		if (last_buffer != NULL)
			free(last_buffer);
		last_buffer = buffer;
		buffer = get_next_line(socket_fd);
	}
	return (last_buffer);
}

char	*ft_read_socket_once(const int socket_fd)
{
	char	*buffer = NULL;

	while ((buffer = get_next_line(socket_fd)) == NULL);

	return (buffer);
}

struct sockaddr_in	ft_init_addr(const int port)
{
	struct sockaddr_in	addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	return (addr);
}
