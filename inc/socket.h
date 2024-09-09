#ifndef SOCKET_H
# define SOCKET_H

# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <errno.h>
# include <stdbool.h>
# include "get_next_line.h"

int					ft_init_socket(struct sockaddr_in addr);
int					ft_send_socket(const int socket_fd, const char *msg);
char				*ft_read_socket(const int socket_fd);
char				*ft_read_socket_once(const int socket_fd);
struct sockaddr_in	ft_init_addr(const char *hostname, const int port);

#endif
