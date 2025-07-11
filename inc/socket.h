#ifndef SOCKET_H
# define SOCKET_H

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

int					ft_init_socket(struct sockaddr_in addr);
int					ft_send_socket(const int socket_fd, const char *msg);
char				*ft_read_socket(const int socket_fd);
char				*ft_read_socket_once(const int socket_fd);
struct sockaddr_in	ft_init_addr(const char *hostname, const int port);

#endif
