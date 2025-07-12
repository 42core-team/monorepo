#include "core_lib_internal.h"

int core_internal_socket_init(struct sockaddr_in addr)
{
	int socket_fd, status_con;
	write(1, "Connecting to server", 21);

	socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (socket_fd < 0)
		fprintf(stderr, "Error: %s: %s\n", __func__, strerror(errno)), exit(EXIT_FAILURE);

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

	write(1, "Connected!\n", 11);
	return socket_fd;
}

int core_internal_socket_send(const int socket_fd, const char *msg)
{
	if (!msg)
		return -1;

	ssize_t status_send = send(socket_fd, msg, strlen(msg), 0);
	if (status_send < 0)
		perror("Socket send failed"), exit(EXIT_FAILURE);

	return (int)status_send;
}


#define BUFFER_SIZE 1024
static char *core_static_socket_readLine(int fd)
{
	static char	*buf;
	char		tmp[BUFFER_SIZE + 1];
	ssize_t		r;
	char		*nl;
	size_t		len;

	if (fd < 0 || BUFFER_SIZE <= 0)
		return (NULL);
	while (!(nl = buf ? strchr(buf, '\n') : NULL)
		&& (r = read(fd, tmp, BUFFER_SIZE)) > 0)
	{
		tmp[r] = '\0';
		size_t old = buf ? strlen(buf) : 0;
		char *ext = realloc(buf, old + r + 1);
		if (!ext)
			return (free(buf), buf = NULL, NULL);
		buf = ext;
		memcpy(buf + old, tmp, r + 1);
	}
	if (!buf || !*buf)
		return (free(buf), buf = NULL, NULL);
	nl = strchr(buf, '\n');
	len = nl ? (size_t)(nl - buf) + 1 : strlen(buf);
	char *line = strndup(buf, len);
	char *rest = strdup(buf + len);
	free(buf);
	buf = rest;
	return (line);
}

static bool core_static_socket_waitForData(int fd)
{
	fd_set readfds;
	struct timeval tv = {10, 0};
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	int retval = select(fd + 1, &readfds, NULL, NULL, &tv);
	if (retval == -1) {
		fprintf(stderr, "Error: %s: %s\n", __func__, strerror(errno));
		return false;
	} else if (retval == 0) {
		fprintf(stderr, "Error: %s: %s\n", __func__, "Did not receive any data from socket fd");
		return false;
	}
	return true;
}

char *core_internal_socket_read(const int socket_fd)
{
	char	*buffer = NULL;
	char	*last_buffer = NULL;

	core_static_socket_waitForData(socket_fd);
	buffer = core_static_socket_readLine(socket_fd);

	while (buffer != NULL)
	{
		if (last_buffer != NULL)
			free(last_buffer);
		last_buffer = buffer;
		buffer = core_static_socket_readLine(socket_fd);
	}
	return (last_buffer);
}

char *core_internal_socket_read_once(const int socket_fd)
{
	core_static_socket_waitForData(socket_fd);
	return core_static_socket_readLine(socket_fd);
}

struct sockaddr_in core_internal_socket_initAddr(const char *hostname, const int port)
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
