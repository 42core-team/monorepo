#include "core_lib_internal.h"

static void ft_print_error(const char *str, const char *func_name)
{
	fprintf(stderr, "Error: %s: %s\n", func_name, str);
}

int ft_init_socket(struct sockaddr_in addr)
{
	int socket_fd, status_con;
	write(1, "Connecting to server", 21);

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		perror("Socket creation failed"), exit(EXIT_FAILURE);

	// Set socket to non-blocking mode
	int flags = fcntl(socket_fd, F_GETFL, 0);
	if (flags < 0)
		perror("Get socket flags failed"), exit(EXIT_FAILURE);

	flags |= O_NONBLOCK;
	if (fcntl(socket_fd, F_SETFL, flags) < 0)
		perror("Set socket to non-blocking mode failed"), exit(EXIT_FAILURE);

	do {
		write(1, ".", 1);
		status_con = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
		if (status_con < 0) {
			fd_set waitset;
			FD_ZERO(&waitset);
			FD_SET(socket_fd, &waitset);

			struct timeval timeout = {5, 0};
			status_con = select(socket_fd + 1, NULL, &waitset, NULL, &timeout);
		}
		if (status_con != 0)
			usleep(350000);
	} while (status_con != 0);

	write(1, "Connected!\n", 12);
	return socket_fd;
}

int ft_send_socket(const int socket_fd, const char *msg)
{
	if (!msg)
		return -1;

	ssize_t status_send = send(socket_fd, msg, strlen(msg), 0);
	if (status_send < 0)
		perror("Socket send failed"), exit(EXIT_FAILURE);

	return (int)status_send;
}

/**
 * @brief Read a line terminated by '\n' from a socket file descriptor.
 * Returns a malloc'ed string without the '\n'. Caller must free.
 */
static char *read_line(int fd)
{
	size_t capacity = 1024;
	size_t len = 0;
	char *buffer = malloc(capacity);
	if (!buffer)
		return NULL;

	while (1) {
		char c;
		ssize_t r = recv(fd, &c, 1, 0);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			free(buffer);
			return NULL;
		}
		if (r == 0)
			break;
		if (c == '\n')
			break;
		if (len + 1 >= capacity) {
			capacity *= 2;
			char *tmp = realloc(buffer, capacity);
			if (!tmp) {
				free(buffer);
				return NULL;
			}
			buffer = tmp;
		}
		buffer[len++] = c;
	}
	buffer[len] = '\0';
	return buffer;
}

bool ft_wait_for_data(int fd)
{
	fd_set readfds;
	struct timeval tv = {10, 0};
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	int retval = select(fd + 1, &readfds, NULL, NULL, &tv);
	if (retval == -1) {
		ft_print_error(strerror(errno), __func__);
		return false;
	} else if (retval == 0) {
		ft_print_error("Did not receive any data from socket fd", __func__);
		return false;
	}
	return true;
}

char *ft_read_socket(const int socket_fd)
{
	if (!ft_wait_for_data(socket_fd))
		return NULL;

	char *line = NULL;
	char *last = NULL;

	while ((line = read_line(socket_fd)) != NULL) {
		free(last);
		last = line;
	}
	return last;
}

char *ft_read_socket_once(const int socket_fd)
{
	if (!ft_wait_for_data(socket_fd))
		return NULL;
	return read_line(socket_fd);
}

struct sockaddr_in ft_init_addr(const char *hostname, const int port)
{
	struct addrinfo hints, *res, *p;
	int status;
	struct sockaddr_in addr;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(EXIT_FAILURE);
	}

	for (p = res; p != NULL; p = p->ai_next) {
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
		addr = *ipv4;
		addr.sin_port = htons(port);
		break;
	}
	freeaddrinfo(res);

	if (p == NULL) {
		fprintf(stderr, "Failed to resolve hostname\n");
		exit(EXIT_FAILURE);
	}
	return addr;
}
