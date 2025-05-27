#include "parse_json.h"

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

	if (game.actions.attacks != NULL)
		free(game.actions.attacks);
	game.actions.attacks = NULL;
	game.actions.attacks_count = 0;

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
	int size = game.actions.travels_count + game.actions.creates_count + game.actions.attacks_count + game.actions.transfer_moneys_count + game.actions.builds_count + 1;
	actions_array->array = malloc(sizeof(json_node*) * size);
	actions_array->array[size - 1] = NULL;

	int i;
	int max = game.actions.travels_count;
	for (i = 0; i < max; i++)
	{
		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 5);
		action->array[4] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("move");
		action->array[0] = type;

		json_node *unit_id = create_node(JSON_TYPE_NUMBER);
		unit_id->key = strdup("unit_id");
		unit_id->number = game.actions.travels[i].id;
		action->array[1] = unit_id;

		json_node *targetX = create_node(JSON_TYPE_NUMBER);
		targetX->key = strdup("targetX");
		targetX->number = game.actions.travels[i].target_pos.x;
		action->array[2] = targetX;

		json_node *targetY = create_node(JSON_TYPE_NUMBER);
		targetY->key = strdup("targetY");
		targetY->number = game.actions.travels[i].target_pos.y;
		action->array[3] = targetY;

		actions_array->array[i] = action;
	}
	max += game.actions.creates_count;
	for (; i < max; i++)
	{
		t_action_create *actionObj = &game.actions.creates[i - game.actions.travels_count];

		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 3);
		action->array[2] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("create");
		action->array[0] = type;

		json_node *type_id = create_node(JSON_TYPE_NUMBER);
		type_id->key = strdup("type_id");
		type_id->number = actionObj->type_id;
		action->array[1] = type_id;

		actions_array->array[i] = action;
	}
	max += game.actions.attacks_count;
	for (; i < max; i++)
	{
		t_action_attack *actionObj = &game.actions.attacks[i - game.actions.travels_count - game.actions.creates_count];

		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 5);
		action->array[4] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("attack");
		action->array[0] = type;

		json_node *unit_id = create_node(JSON_TYPE_NUMBER);
		unit_id->key = strdup("unit_id");
		unit_id->number = actionObj->id;
		action->array[1] = unit_id;

		json_node *target_id = create_node(JSON_TYPE_NUMBER);
		target_id->key = strdup("target_id_x");
		target_id->number = actionObj->target_pos.x;
		action->array[2] = target_id;

		json_node *target_y = create_node(JSON_TYPE_NUMBER);
		target_y->key = strdup("target_id_y");
		target_y->number = actionObj->target_pos.y;
		action->array[3] = target_y;

		actions_array->array[i] = action;
	}
	max += game.actions.transfer_moneys_count;
	for (; i < max; i++)
	{
		t_action_transfer_money *actionObj = &game.actions.transfer_moneys[i - game.actions.travels_count - game.actions.creates_count - game.actions.attacks_count];

		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 6);
		action->array[5] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("transfer_money");
		action->array[0] = type;

		json_node *source_id = create_node(JSON_TYPE_NUMBER);
		source_id->key = strdup("source_id");
		source_id->number = actionObj->source_id;
		action->array[1] = source_id;

		json_node *target_x = create_node(JSON_TYPE_NUMBER);
		target_x->key = strdup("x");
		target_x->number = actionObj->target_pos.x;
		action->array[2] = target_x;

		json_node *target_y = create_node(JSON_TYPE_NUMBER);
		target_y->key = strdup("y");
		target_y->number = actionObj->target_pos.y;
		action->array[3] = target_y;

		json_node *amount = create_node(JSON_TYPE_NUMBER);
		amount->key = strdup("amount");
		amount->number = actionObj->amount;
		action->array[4] = amount;

		actions_array->array[i] = action;
	}
	max += game.actions.builds_count;
	for (; i < max; i++)
	{
		t_action_build *actionObj = &game.actions.builds[i - game.actions.travels_count - game.actions.creates_count - game.actions.transfer_moneys_count - game.actions.attacks_count];

		json_node *action = create_node(JSON_TYPE_OBJECT);
		action->array = malloc(sizeof(json_node*) * 5);
		action->array[4] = NULL;

		json_node *type = create_node(JSON_TYPE_STRING);
		type->key = strdup("type");
		type->string = strdup("build");
		action->array[0] = type;

		json_node *type_id = create_node(JSON_TYPE_NUMBER);
		type_id->key = strdup("builder_id");
		type_id->number = actionObj->id;
		action->array[1] = type_id;

		json_node *x = create_node(JSON_TYPE_NUMBER);
		x->key = strdup("x");
		x->number = actionObj->pos.x;
		action->array[2] = x;

		json_node *y = create_node(JSON_TYPE_NUMBER);
		y->key = strdup("y");
		y->number = actionObj->pos.y;
		action->array[3] = y;

		actions_array->array[i] = action;
	}

	json_node *root_obj = create_node(JSON_TYPE_OBJECT);
	root_obj->array = malloc(sizeof(json_node*) * 2);
	root_obj->array[1] = NULL;

	json_node *actions_key = create_node(JSON_TYPE_ARRAY);
	actions_key->key = strdup("actions");
	actions_key->array = actions_array->array;

	root_obj->array[0] = actions_key;

	char *json = json_to_string(root_obj);
	free_json(root_obj);

	int json_len = strlen(json);
	json = realloc(json, json_len + 2);
	json[json_len] = '\n';
	json[json_len + 1] = '\0';

	return json;
}
