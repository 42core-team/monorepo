#ifndef CORE_LIB_INTERNAL_H
#define CORE_LIB_INTERNAL_H

#include "core_lib.h"

// ----- General

void ft_free_game();
void ft_perror_exit(char *msg);

int core_util_distance(t_pos pos1, t_pos pos2);

// ----- Socket

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

// ----- Actions

typedef enum e_action_type
{
	ACTION_CREATE,
	ACTION_MOVE,
	ACTION_ATTACK,
	ACTION_TRANSFER,
	ACTION_BUILD
} t_action_type;

typedef struct s_action
{
	t_action_type type;
	union
	{
		struct { unsigned long unit_type; } create;
		struct { unsigned long id; t_pos pos; } move;
		struct { unsigned long id; t_pos pos; } attack;
		struct { unsigned long source_id; t_pos target_pos; unsigned long amount; } transfer;
		struct { unsigned long builder_id; t_pos pos; } build;
	} data;
} t_action;

typedef struct s_actions
{
	t_action *list;
	unsigned int count;
	unsigned int capacity;
} t_actions;

extern t_actions actions;

// ----- JSON parsing and encoding

void	ft_parse_json_state(char *json);
void	ft_parse_json_config(char *json);
char	*ft_all_action_json();
void	ft_reset_actions();
char	*ft_create_login_msg(char *team_name, int argc, char **argv);

// ----- JSON LIB

#include <ctype.h>

// --- Basic Structs

typedef enum e_json_type
{
	JSON_TYPE_NULL,
	JSON_TYPE_BOOL,
	JSON_TYPE_STRING,
	JSON_TYPE_NUMBER,
	JSON_TYPE_OBJECT,
	JSON_TYPE_ARRAY
} json_type;

typedef struct s_json_node
{
	char *key;
	json_type type;

	union
	{
		char *string;
		double number;
		struct s_json_node **array;
	};
}	json_node;

// --- Functions

json_node *string_to_json(char *string);					// Convert a JSON string into a JSON tree
char *json_to_string(json_node *json);						// Convert a JSON tree to a JSON string
char *json_to_formatted_string(json_node *json);			// Convert a JSON tree to a formatted JSON string
json_node *json_find(json_node *json, char *key);			// Find a node in the JSON tree by key (top-level only)
json_node *json_find_recursive(json_node *json, char *key);	// Find a node in the JSON tree by key (recursive)
void free_json(json_node *json);							// Free all memory allocated for the JSON tree
json_node *create_node(json_type type);						// Create a new, empty-initialized node

#endif // CORE_LIB_INTERNAL_H