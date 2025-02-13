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

	if (game.actions.transfer_moneys != NULL)
		free(game.actions.transfer_moneys);
	game.actions.transfer_moneys = NULL;
	game.actions.transfer_moneys_count = 0;

	if (game.actions.builds != NULL)
		free(game.actions.builds);
	game.actions.builds = NULL;
	game.actions.builds_count = 0;
}

char *ft_all_action_json(void)
{
	json_node *actions_array = create_node(JSON_TYPE_ARRAY);
	int size = game.actions.travels_count + game.actions.creates_count + game.actions.transfer_moneys_count + game.actions.builds_count + 1;
	actions_array->array = malloc(sizeof(json_node*) * size);
	actions_array->array[size - 1] = NULL;

	int i;
	int max = game.actions.travels_count;
	for (i = 0; i < max; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 4);
		action->array[3] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("move");
		action->array[0] = type;

		json_node *unit_id = create_node(JSON_TYPE_NUMBER);
		unit_id->key = strdup("unit_id");
		unit_id->number = game.actions.travels[i].id;
		action->array[1] = unit_id;

		json_node *dir = create_node(JSON_TYPE_STRING);
		dir->key = strdup("dir");
		dir->string = strdup(ft_direction_to_str(game.actions.travels[i].direction));
		action->array[2] = dir;

		actions_array->array[i] = action;
	}
	max += game.actions.creates_count;
	for (; i < max; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 3);
		action->array[2] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("create");
		action->array[0] = type;

		json_node *type_id = create_node(JSON_TYPE_NUMBER);
		type_id->key = strdup("type_id");
		type_id->number = game.actions.creates[i - game.actions.travels_count].type_id;
		action->array[1] = type_id;

		actions_array->array[i] = action;
	}
	max += game.actions.transfer_moneys_count;
	for (; i < max; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 5);
		action->array[4] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("transfer_money");
		action->array[0] = type;

		json_node *source_id = create_node(JSON_TYPE_NUMBER);
		source_id->key = strdup("source_id");
		source_id->number = game.actions.transfer_moneys[i - game.actions.travels_count - game.actions.creates_count].source_id;
		action->array[1] = source_id;

		json_node *target_id = create_node(JSON_TYPE_NUMBER);
		target_id->key = strdup("target_id");
		target_id->number = game.actions.transfer_moneys[i - game.actions.travels_count - game.actions.creates_count].target_id;
		action->array[2] = target_id;

		json_node *amount = create_node(JSON_TYPE_NUMBER);
		amount->key = strdup("amount");
		amount->number = game.actions.transfer_moneys[i - game.actions.travels_count - game.actions.creates_count].amount;
		action->array[3] = amount;

		actions_array->array[i] = action;
	}
	max += game.actions.builds_count;
	for (; i < max; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 5);
		action->array[4] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("build");
		action->array[0] = type;

		json_node *type_id = create_node(JSON_TYPE_NUMBER);
		type_id->key = strdup("builder_id");
		type_id->number = game.actions.builds[i - game.actions.travels_count - game.actions.creates_count - game.actions.transfer_moneys_count].id;
		action->array[1] = type_id;

		json_node *x = create_node(JSON_TYPE_NUMBER);
		x->key = strdup("x");
		x->number = game.actions.builds[i - game.actions.travels_count - game.actions.creates_count - game.actions.transfer_moneys_count].pos.x;
		action->array[2] = x;

		json_node *y = create_node(JSON_TYPE_NUMBER);
		y->key = strdup("y");
		y->number = game.actions.builds[i - game.actions.travels_count - game.actions.creates_count - game.actions.transfer_moneys_count].pos.y;
		action->array[3] = y;

		actions_array->array[i] = action;
	}

	json_node *root_obj = create_node(JSON_TYPE_OBJECT);
	root_obj->array = malloc(sizeof(json_node*) * 2);
	root_obj->array[1] = NULL;

	json_node *actions_key = create_node(JSON_TYPE_ARRAY);
	actions_key->key = strdup("actions");
	actions_key->array = actions_array->array;
	free(actions_array);

	root_obj->array[0] = actions_key;

	char *json = json_to_string(root_obj);
	free_json(root_obj);

	int json_len = strlen(json);
	json = realloc(json, json_len + 2);
	json[json_len] = '\n';
	json[json_len + 1] = '\0';

	return json;
}
