#include "parse_json.h"

char	*ft_create_login_msg(char *team_name, int *argc, char **argv)
{
	(void)argc;

	json_node * root = create_node(JSON_TYPE_OBJECT);
	root->array = malloc(sizeof(json_node *) * 4);
	root->array[3] = NULL;

	json_node *password = create_node(JSON_TYPE_STRING);
	password->string = strdup("42");
	password->key = strdup("password");
	root->array[0] = password;

	json_node *id = create_node(JSON_TYPE_NUMBER);
	id->number = argv[1] ? atoi(argv[1]) : -1;
	id->key = strdup("id");
	root->array[1] = id;

	json_node *name = create_node(JSON_TYPE_STRING);
	name->string = team_name ? strdup(team_name) : strdup("default");
	name->key = strdup("name");
	root->array[2] = name;

	char * msg = json_to_string(root);
	free_json(root);

	int msg_len = strlen(msg);
	msg = realloc(msg, msg_len + 2);
	msg[msg_len] = '\n';
	msg[msg_len + 1] = '\0';

	return (msg);
}
