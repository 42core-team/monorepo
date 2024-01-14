#include "socket.h"

int	ft_init_socket(struct sockaddr_in addr)
{
	int	socket_fd, status_con;

	write(1, "Connecting to server", 21);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
	{
		perror("Socket creation failed");
		exit(1);
	}

	// Set socket to non-blocking mode
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0)
	{
		perror("Get socket flags failed");
		exit(1);
	}
	flags |= O_NONBLOCK;
	if (fcntl(socket_fd, F_SETFL, flags) < 0)
	{
		perror("Set socket to non-blocking mode failed");
		exit(1);
	}

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
			sleep(1);
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
	{
		perror("Socket send failed");
		exit(1);
	}
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

struct sockaddr_in	ft_init_addr(const char ip[10], const int port)
{
	struct sockaddr_in	addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		perror("IP address invalid");
		exit(1);
	}
	return (addr);
}
