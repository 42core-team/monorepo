#include "socket.h"
#include "con_lib.h"

#include <netdb.h>
#include <arpa/inet.h>

static void ft_print_error(char *str, const char *func_name)
{
	fprintf(stderr, "Error: %s: %s\n", func_name, str);
}

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

bool ft_wait_for_data(int fd) {
	fd_set readfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	tv.tv_sec = 10;
	tv.tv_usec = 0;

	retval = select(fd + 1, &readfds, NULL, NULL, &tv);

	if (retval == -1) {
		ft_print_error(strerror(errno), __func__);
		return false;
	} else if (retval) {
		return true;
	} else {
		ft_print_error("Did not receive any data from socket fd", __func__);
		return false;
	}
}

char	*ft_read_socket(const int socket_fd)
{
	char	*buffer = NULL;
	char	*last_buffer = NULL;

	ft_wait_for_data(socket_fd);
	buffer = get_next_line(socket_fd);

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

	ft_wait_for_data(socket_fd);
	buffer = get_next_line(socket_fd);

	return (buffer);
}

struct sockaddr_in	ft_init_addr(const char *hostname, const int port)
{
	struct addrinfo hints, *res, *p;
	int status;
	struct sockaddr_in addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // AF_INET for IPv4, AF_INET6 for IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	// Loop through all the results and use the first one we can
	for(p = res; p != NULL; p = p->ai_next) {
		// Cast the sockaddr to sockaddr_in to access the sin_addr field
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr = ipv4->sin_addr; // Copy the address
		break; // If we get here, we have our address
	}

	freeaddrinfo(res); // Free the linked list

	if (p == NULL) {
		// We didn't find any address
		fprintf(stderr, "Failed to resolve hostname\n");
		exit(EXIT_FAILURE);
	}

	return addr;
}
