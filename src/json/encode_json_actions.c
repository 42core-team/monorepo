#include "parse_json.h"

static char *ft_direction_to_str(t_direction dir)
{
	switch (dir)
	{
		case DIR_UP:	return "u";
		case DIR_RIGHT:	return "r";
		case DIR_DOWN:	return "d";
		case DIR_LEFT:	return "l";
		default:		return "u";
	}
}

void	ft_reset_actions()
{
	if (game.actions.creates != NULL)
		free(game.actions.creates);
	game.actions.creates = NULL;
	game.actions.creates_count = 0;

	if (game.actions.travels != NULL)
		free(game.actions.travels);
	game.actions.travels = NULL;
	game.actions.travels_count = 0;
}

char *ft_all_action_json(void)
{
	json_node *actions = create_node(JSON_TYPE_ARRAY);
	actions->string = strdup("actions");
	int size = game.actions.travels_count + game.actions.creates_count + 1;
	actions->array = malloc(sizeof(json_node*) * size);
	actions->array[size - 1] = NULL;

	int i;
	for (i = 0; i < (int)game.actions.travels_count; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 4);
		action->array[3] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("move");
		action->array[0] = type;

		json_node *unit_id = create_node(JSON_TYPE_STRING);
		unit_id->key = strdup("unit_id");
		unit_id->string = ft_direction_to_str(game.actions.travels[i].direction);
		action->array[1] = unit_id;

		json_node *dir = create_node(JSON_TYPE_STRING);
		dir->key = strdup("dir");
		dir->string = strdup(ft_direction_to_str(game.actions.travels[i].direction));
		action->array[2] = dir;

		actions->array[i] = action;
	}
	for (; i < (int)game.actions.travels_count + (int)game.actions.creates_count; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 2);
		action->array[1] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("create");
		action->array[0] = type;

		json_node *type_id = create_node(JSON_TYPE_STRING);
		type_id->key = strdup("type_id");
		type_id->string = malloc(2);
		type_id->string[0] = game.actions.creates[i - game.actions.travels_count].type_id + '0';
		type_id->string[1] = '\0';
		action->array[1] = type_id;

		actions->array[i] = action;
	}

	char *json = json_to_string(actions);
	free_json(actions);

	int json_len = strlen(json);
	json = realloc(json, json_len + 2);
	json[json_len] = '\n';
	json[json_len + 1] = '\0';

	return json;
}
