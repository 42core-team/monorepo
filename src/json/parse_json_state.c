#include "parse_json.h"

static void apply_obj_to_arr(t_obj obj, t_obj ***arr)
{
	bool objInserted = false;

	obj.state = STATE_ALIVE;

	// 1. LOOP: Id Matching
	size_t index = 0;
	while ((*arr)[index] != NULL)
	{
		if ((*arr)[index]->id == obj.id)
		{
			t_obj * existingObj = (*arr)[index];
			existingObj->type = obj.type;
			existingObj->state = STATE_ALIVE;
			existingObj->id = obj.id;
			existingObj->pos = obj.pos;
			existingObj->hp = obj.hp;
			if ((*arr) == game.units)
			{
				existingObj->s_unit.type_id = obj.s_unit.type_id;
				existingObj->s_unit.team_id = obj.s_unit.team_id;
			}
			if ((*arr) == game.cores)
			{
				existingObj->s_core.team_id = obj.s_core.team_id;
				existingObj->s_core.balance = obj.s_core.balance;
			}
			objInserted = true;
			break;
		}
		index++;
	}
	if (objInserted)
		return ;

	// 2. LOOP: Placeholder matching
	index = 0;
	while ((*arr)[index] != NULL)
	{
		bool matches = (*arr)[index]->id == 0;
		if ((*arr) == game.units && (((*arr)[index]->s_unit.type_id != obj.s_unit.type_id) || ((*arr)[index]->s_unit.team_id != obj.s_unit.team_id)))
			matches = false;
		if ((*arr) == game.cores && (*arr)[index]->s_core.team_id != obj.s_core.team_id)
			matches = false;

		if (matches)
		{
			t_obj * existingObj = (*arr)[index];
			existingObj->type = obj.type;
			existingObj->state = STATE_ALIVE;
			existingObj->id = obj.id;
			existingObj->pos = obj.pos;
			existingObj->hp = obj.hp;
			if ((*arr) == game.units)
			{
				existingObj->s_unit.type_id = obj.s_unit.type_id;
				existingObj->s_unit.team_id = obj.s_unit.team_id;
			}
			if ((*arr) == game.cores)
			{
				existingObj->s_core.team_id = obj.s_core.team_id;
				existingObj->s_core.balance = obj.s_core.balance;
			}
			objInserted = true;
			break;
		}
		index++;
	}
	if (objInserted)
		return ;

	if ((*arr) == game.units && obj.s_unit.team_id == game.my_team_id)
	{
		printf("Error matching team units. This is never supposed to happen. Troublemaker: [id %lu, type %lu]\n", obj.id, obj.s_unit.type_id);
	}

	// 3. Add to the back
	size_t arrLen = 0;
	while ((*arr)[arrLen] != NULL)
		arrLen++;
	(*arr) = realloc((*arr), sizeof(t_obj *) * (arrLen + 2));
	(*arr)[arrLen + 1] = NULL;
	(*arr)[arrLen] = malloc(sizeof(t_obj));
	t_obj * existingObj = (*arr)[arrLen];
	existingObj->type = obj.type;
	existingObj->state = STATE_ALIVE;
	existingObj->id = obj.id;
	existingObj->pos = obj.pos;
	existingObj->hp = obj.hp;
	existingObj->data = NULL;
	if ((*arr) == game.units)
	{
		existingObj->s_unit.type_id = obj.s_unit.type_id;
		existingObj->s_unit.team_id = obj.s_unit.team_id;
	}
	if ((*arr) == game.cores)
	{
		existingObj->s_core.team_id = obj.s_core.team_id;
		existingObj->s_core.balance = obj.s_core.balance;
	}
}

static void ft_parse_object(json_node * json, t_obj_type type, t_obj ** game_arr)
{
	if (game_arr == NULL)
	{
		game_arr = malloc(sizeof(t_obj *) * 1);
		game_arr[0] = NULL;
	}

	for (size_t i = 0; game_arr[i] != NULL; i++)
		if (game_arr[i]->state == STATE_ALIVE)
			game_arr[i]->state = STATE_DEAD;

	for (int i = 0; json->array[i] != NULL; i++)
	{
		t_obj readObj;
		readObj.type = type;
		readObj.id = (unsigned long)json_find(json_find(json->array[i], "id"), "number")->number;
		readObj.pos.x = (unsigned short)json_find(json_find(json->array[i], "x"), "number")->number;
		readObj.pos.y = (unsigned short)json_find(json_find(json->array[i], "y"), "number")->number;
		readObj.hp = (unsigned long)json_find(json_find(json->array[i], "hp"), "number")->number;

		if (type == OBJ_UNIT)
		{
			readObj.s_unit.type_id = (unsigned long)json_find(json_find(json->array[i], "type"), "number")->number;
			readObj.s_unit.team_id = (unsigned long)json_find(json_find(json->array[i], "teamId"), "number")->number;
			readObj.s_unit.balance = (unsigned long)json_find(json_find(json->array[i], "balance"), "number")->number;
		}
		if (type == OBJ_CORE)
		{
			readObj.s_core.team_id = (unsigned long)json_find(json_find(json->array[i], "teamId"), "number")->number;
			readObj.s_core.balance = (unsigned long)json_find(json_find(json->array[i], "balance"), "number")->number;
		}

		apply_obj_to_arr(readObj, &game_arr);
	}
}

void	ft_parse_json_state(char *json)
{
	json_node * root = string_to_json(json);
	
	ft_parse_object(json_find(root, "cores"), OBJ_CORE, game.cores);
	ft_parse_object(json_find(root, "resources"), OBJ_RESOURCE, game.resources);
	ft_parse_object(json_find(root, "units"), OBJ_UNIT, game.units);
	ft_parse_object(json_find(root, "walls"), OBJ_WALL, game.walls);

	free_json(root);
}
