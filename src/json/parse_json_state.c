#include "parse_json.h"

static void apply_obj_to_arr(t_obj obj)
{
	if (game.objects == NULL)
	{
		game.objects = malloc(sizeof(t_obj *) * 1);
		if (!game.objects)
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}
		game.objects[0] = NULL;
	}

	obj.state = STATE_ALIVE;

	// 1. LOOP: Id Matching
	for (size_t index = 0; game.objects[index] != NULL; index++)
	{
		if (game.objects[index]->id != obj.id)
			continue;

		t_obj *existingObj = game.objects[index];

		existingObj->type = obj.type;
		existingObj->state = STATE_ALIVE;
		existingObj->id = obj.id;
		existingObj->pos = obj.pos;
		existingObj->hp = obj.hp;
		switch (obj.type)
		{
			case OBJ_UNIT:
				existingObj->s_unit = obj.s_unit;
				break;
			case OBJ_CORE:
				existingObj->s_core = obj.s_core;
				break;
			case OBJ_RESOURCE:
			case OBJ_MONEY:
				existingObj->s_resource_money = obj.s_resource_money;
				break;
			case OBJ_BOMB:
				existingObj->s_bomb = obj.s_bomb;
				break;
			default:
				break;
		}

		return;
	}

	// 2. LOOP: Placeholder matching
	for (size_t index = 0; game.objects[index] != NULL; index++)
	{
		if (obj.type != OBJ_UNIT)
			continue;

		if ((game.objects[index]->s_unit.unit_type != obj.s_unit.unit_type) || (game.objects[index]->s_unit.team_id != obj.s_unit.team_id))
			continue;

		t_obj *existingObj = game.objects[index];
		existingObj->type = obj.type;
		existingObj->state = STATE_ALIVE;
		existingObj->id = obj.id;
		existingObj->pos = obj.pos;
		existingObj->hp = obj.hp;
		switch (obj.type)
		{
			case OBJ_UNIT:
				existingObj->s_unit = obj.s_unit;
				break;
			case OBJ_CORE:
				existingObj->s_core = obj.s_core;
				break;
			case OBJ_RESOURCE:
			case OBJ_MONEY:
				existingObj->s_resource_money = obj.s_resource_money;
				break;
			case OBJ_BOMB:
				existingObj->s_bomb = obj.s_bomb;
				break;
			default:
				break;
		}
		return;
	}

	if (obj.type == OBJ_UNIT && obj.s_unit.team_id == game.my_team_id)
	{
		printf("Error matching team units. This is never supposed to happen. Have you freed something you shouldn't have? Never free t_obj*s, just read them. Troublemaker: [id %lu, unit_type %lu, team %lu]\n", obj.id, obj.s_unit.unit_type, obj.s_unit.team_id);
	}

	// 3. Add to the back
	size_t arrLen = 0;
	while (game.objects[arrLen] != NULL)
		arrLen++;
	game.objects = realloc(game.objects, sizeof(t_obj *) * (arrLen + 2));
	game.objects[arrLen + 1] = NULL;
	game.objects[arrLen] = malloc(sizeof(t_obj));
	t_obj *newObj = game.objects[arrLen];
	*newObj = obj;
	newObj->state = STATE_ALIVE;
}

void ft_parse_json_state(char *json)
{
	json_node *root = string_to_json(json);

	game.elapsed_ticks = (unsigned long)json_find(root, "tick")->number;

	json_node *objects = json_find(root, "objects");

	if (game.objects != NULL)
		for (size_t i = 0; game.objects[i] != NULL; i++)
			if (game.objects[i]->state == STATE_ALIVE)
				game.objects[i]->state = STATE_DEAD;

	for (int i = 0; objects->array != NULL && objects->array[i] != NULL; i++)
	{
		t_obj readObj;
		readObj.id = (unsigned long)json_find(objects->array[i], "id")->number;
		readObj.type = (t_obj_type)json_find(objects->array[i], "type")->number;
		readObj.pos.x = (unsigned short)json_find(objects->array[i], "x")->number;
		readObj.pos.y = (unsigned short)json_find(objects->array[i], "y")->number;
		readObj.hp = (unsigned long)json_find(objects->array[i], "hp")->number;

		// object-specific fields (wall doesn't have any)
		if (readObj.type == OBJ_CORE)
		{
			readObj.s_core.team_id = (unsigned long)json_find(objects->array[i], "teamId")->number;
			readObj.s_core.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
		}
		if (readObj.type == OBJ_UNIT)
		{
			readObj.s_unit.unit_type = (unsigned long)json_find(objects->array[i], "unit_type")->number;
			readObj.s_unit.team_id = (unsigned long)json_find(objects->array[i], "teamId")->number;
			readObj.s_unit.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
			readObj.s_unit.next_movement_opp = (unsigned long)json_find(objects->array[i], "nextMoveOpp")->number;
		}
		if (readObj.type == OBJ_RESOURCE || readObj.type == OBJ_MONEY)
		{
			readObj.s_resource_money.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
		}
		if (readObj.type == OBJ_BOMB)
		{
			readObj.s_bomb.countdown = (unsigned long)json_find(objects->array[i], "countdown")->number;
		}

		apply_obj_to_arr(readObj);
	}

	// clean uninitialized units, as the server did not send them
	if (game.objects && game.objects[0])
	{
		int j = 0;
		for (size_t i = 0; game.objects[i] != NULL; i++)
		{
			t_obj *obj = game.objects[i];
			if (obj->type == OBJ_UNIT && obj->state == STATE_UNINITIALIZED) {
				free(obj);
				continue;
			}
			game.objects[j++] = obj;
		}
		game.objects[j] = NULL;
	}

	free_json(root);
}
