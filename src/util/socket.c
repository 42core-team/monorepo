#include "socket.h"

void	ft_perror_exit(char *msg);

static FILE  *sock_stream = NULL;

int ft_init_socket(struct sockaddr_in addr)
{
	int socket_fd, flags, status_con;

	write(1, "Connecting to server", 21);
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		ft_perror_exit("Socket creation failed");

	/* set non-blocking */
	if ((flags = fcntl(socket_fd, F_GETFL, 0)) < 0 ||
		fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		ft_perror_exit("Failed to set non-blocking mode");

	/* attempt connect (with timeout loop) */
	do {
		write(1, ".", 1);
		status_con = connect(socket_fd, (struct sockaddr *)&addr, sizeof(addr));
		if (status_con < 0) {
			fd_set ws;
			struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };
			FD_ZERO(&ws);
			FD_SET(socket_fd, &ws);
			status_con = select(socket_fd + 1, NULL, &ws, NULL, &tv);
		}
		if (status_con != 0)
			usleep(350000);
	} while (status_con != 0);

	write(1, "Connected!\n", 12);

	/* wrap socket in a FILE* for stdio reads */
	sock_stream = fdopen(socket_fd, "r+");
	if (!sock_stream)
		ft_perror_exit("fdopen failed");

	/* Optional: make stdio I/O unbuffered so you don’t hang on fgets/getline */
	setvbuf(sock_stream, NULL, _IONBF, 0);

	return socket_fd;
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

bool ft_wait_for_data(int fd)
{
	fd_set rf;
	struct timeval tv = { .tv_sec = 10, .tv_usec = 0 };
	int retval;

	FD_ZERO(&rf);
	FD_SET(fd, &rf);
	retval = select(fd + 1, &rf, NULL, NULL, &tv);
	if (retval < 0) {
		fprintf(stderr, "Error: %s: %s\n", __func__, strerror(errno));
		return false;
	}
	if (retval == 0) {
		fprintf(stderr, "Error: %s: timeout waiting for data\n", __func__);
		return false;
	}
	return true;
}

char *ft_read_socket(const int socket_fd)
{
	char   *line = NULL;
	size_t  len  = 0;
	char   *last = NULL;
	ssize_t n;

	while (ft_wait_for_data(socket_fd)
		   && (n = getline(&line, &len, sock_stream)) > 0)
	{
		free(last);
		/* duplicate so caller owns it */
		last = strdup(line);
	}
	free(line);
	return last;  /* NULL if no data, or last line read */
}

char *ft_read_socket_once(const int socket_fd)
{
	char   *line = NULL;
	size_t  len  = 0;
	ssize_t n;

	if (ft_wait_for_data(socket_fd)
		&& (n = getline(&line, &len, sock_stream)) > 0)
	{
		return line;  /* caller must free() */
	}
	free(line);
	return NULL;
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
