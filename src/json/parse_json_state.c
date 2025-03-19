#include "parse_json.h"
#include "event_handler.h"

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

			t_obj_state oldState = existingObj->state;
			t_pos oldPos = existingObj->pos;
			unsigned long oldHp = existingObj->hp;
			unsigned long oldBalance = 0;
			if ((*arr) == game.units)
				oldBalance = existingObj->s_unit.balance;
			if ((*arr) == game.cores)
				oldBalance = existingObj->s_core.balance;
			if ((*arr) == game.resources)
				oldBalance = existingObj->s_resource.balance;

			existingObj->type = obj.type;
			existingObj->state = STATE_ALIVE;
			existingObj->id = obj.id;
			existingObj->pos = obj.pos;
			existingObj->hp = obj.hp;
			if ((*arr) == game.units)
			{
				existingObj->s_unit.type_id = obj.s_unit.type_id;
				existingObj->s_unit.team_id = obj.s_unit.team_id;
				existingObj->s_unit.balance = obj.s_unit.balance;
				existingObj->s_unit.next_movement_opp = obj.s_unit.next_movement_opp;
			}
			if ((*arr) == game.cores)
			{
				existingObj->s_core.team_id = obj.s_core.team_id;
				existingObj->s_core.balance = obj.s_core.balance;
			}
			if ((*arr) == game.resources)
			{
				existingObj->s_resource.balance = obj.s_resource.balance;
			}

			if (event_handler.on_object_state_change && oldState != existingObj->state)
				event_handler.on_object_state_change(existingObj, oldState, existingObj->state, user_data);
			if (event_handler.on_object_pos_change && (oldPos.x != existingObj->pos.x || oldPos.y != existingObj->pos.y))
				event_handler.on_object_pos_change(existingObj, oldPos, existingObj->pos, user_data);
			if (event_handler.on_object_health_change && oldHp != existingObj->hp)
				event_handler.on_object_health_change(existingObj, oldHp, existingObj->hp, user_data);
			if (event_handler.on_object_balance_change)
			{
				unsigned long new_balance = 0;
				if ((*arr) == game.units)
					new_balance = existingObj->s_unit.balance;
				else if ((*arr) == game.cores)
					new_balance = existingObj->s_core.balance;
				else if ((*arr) == game.resources)
					new_balance = existingObj->s_resource.balance;
				if (oldBalance != new_balance)
					event_handler.on_object_balance_change(existingObj, oldBalance, new_balance, user_data);
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
				existingObj->s_unit.balance = obj.s_unit.balance;
				existingObj->s_unit.next_movement_opp = obj.s_unit.next_movement_opp;
			}
			if ((*arr) == game.cores)
			{
				existingObj->s_core.team_id = obj.s_core.team_id;
				existingObj->s_core.balance = obj.s_core.balance;
			}
			if ((*arr) == game.resources)
			{
				existingObj->s_resource.balance = obj.s_resource.balance;
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
		printf("Error matching team units. This is never supposed to happen. Have you freed something you shouldn't have? Troublemaker: [id %lu, type %lu, team %lu]\n", obj.id, obj.s_unit.type_id, obj.s_unit.team_id);
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
		existingObj->s_unit.balance = obj.s_unit.balance;
		existingObj->s_unit.next_movement_opp = obj.s_unit.next_movement_opp;
	}
	if ((*arr) == game.cores)
	{
		existingObj->s_core.team_id = obj.s_core.team_id;
		existingObj->s_core.balance = obj.s_core.balance;
	}
	if ((*arr) == game.resources)
	{
		existingObj->s_resource.balance = obj.s_resource.balance;
	}
}

void	ft_parse_json_state(char *json)
{
	json_node * root = string_to_json(json);

	parse_json_actions(root);

	game.elapsed_ticks = (unsigned long)json_find(root, "tick")->number;

	json_node * objects = json_find(root, "objects");

	if (game.cores != NULL)
	{
		for (size_t i = 0; game.cores[i] != NULL; i++)
			if (game.cores[i]->state == STATE_ALIVE)
				game.cores[i]->state = STATE_DEAD;
	}
	if (game.units != NULL)
	{
		for (size_t i = 0; game.units[i] != NULL; i++)
			if (game.units[i]->state == STATE_ALIVE)
				game.units[i]->state = STATE_DEAD;
	}
	if (game.resources != NULL)
	{
		for (size_t i = 0; game.resources[i] != NULL; i++)
			if (game.resources[i]->state == STATE_ALIVE)
				game.resources[i]->state = STATE_DEAD;
	}
	if (game.walls != NULL)
	{
		for (size_t i = 0; game.walls[i] != NULL; i++)
			if (game.walls[i]->state == STATE_ALIVE)
				game.walls[i]->state = STATE_DEAD;
	}

	for (int i = 0; objects->array != NULL && objects->array[i] != NULL; i++)
	{
		t_obj readObj;
		readObj.id = (unsigned long)json_find(objects->array[i], "id")->number;
		readObj.type = (t_obj_type)json_find(objects->array[i], "type")->number;
		readObj.pos.x = (unsigned short)json_find(objects->array[i], "x")->number;
		readObj.pos.y = (unsigned short)json_find(objects->array[i], "y")->number;
		readObj.hp = (unsigned long)json_find(objects->array[i], "hp")->number;

		if (readObj.type == OBJ_CORE)
		{
			readObj.s_core.team_id = (unsigned long)json_find(objects->array[i], "teamId")->number;
			readObj.s_core.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
		}
		if (readObj.type == OBJ_UNIT)
		{
			readObj.s_unit.type_id = (unsigned long)json_find(objects->array[i], "typeId")->number;
			readObj.s_unit.team_id = (unsigned long)json_find(objects->array[i], "teamId")->number;
			readObj.s_unit.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
			readObj.s_unit.next_movement_opp = (unsigned long)json_find(objects->array[i], "nextMoveOpp")->number;
		}
		if (readObj.type == OBJ_RESOURCE)
		{
			readObj.s_resource.balance = (unsigned long)json_find(objects->array[i], "balance")->number;
		}

		t_obj *** game_arr = NULL;
		if (readObj.type == OBJ_CORE)
			game_arr = &game.cores;
		else if (readObj.type == OBJ_UNIT)
			game_arr = &game.units;
		else if (readObj.type == OBJ_RESOURCE)
			game_arr = &game.resources;
		else if (readObj.type == OBJ_WALL)
			game_arr = &game.walls;
		else if (readObj.type == OBJ_MONEY)
			game_arr = &game.moneys;

		if (*game_arr == NULL)
		{
			*game_arr = malloc(sizeof(t_obj *) * 1);
			(*game_arr)[0] = NULL;
		}

		apply_obj_to_arr(readObj, game_arr);
	}

	// clean uninitialized units, as the server did not send them
	if (game.units && game.units[0])
	{
		int j = 0;
		for (size_t i = 0; game.units[i] != NULL; i++)
		{
			if (game.units[i]->state != STATE_UNINITIALIZED)
				game.units[j++] = game.units[i];
			else
				free(game.units[i]);
		}
		game.units[j] = NULL;
	}


	free_json(root);
}
