#include "core_lib_internal.h"

static void core_static_updateObj(t_obj *existingObj, json_node *updates)
{
	if (updates->type != JSON_TYPE_OBJECT)
	{
		char *updatesFormatted = json_to_string(updates);
		printf("Error: Supplied json node is not an array. Got: %s\n", updatesFormatted);
		free (updatesFormatted);
		return;
	}

	if (existingObj->state == STATE_UNINITIALIZED)
		existingObj->state = STATE_ALIVE; // units come alive on first update

	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *property = updates->array[i];

		// even non-changing properties are included so the function can also fully set all fields of an empty obj
		if (strncmp(property->key, "state", 5) == 0)
		{
			if (strncmp(property->string, "dead", 4) == 0)
				existingObj->state = STATE_DEAD;
		}
		else if (strncmp(property->key, "x", 1) == 0)
			existingObj->pos.x = property->number;
		else if (strncmp(property->key, "y", 1) == 0)
			existingObj->pos.y = property->number;
		else if (strncmp(property->key, "hp", 2) == 0)
			existingObj->hp = property->number;
		else if (strncmp(property->key, "type", 4) == 0)
			existingObj->type = property->number;
		else if (strncmp(property->key, "id", 2) == 0)
			existingObj->id = property->number;
		else if (strncmp(property->key, "unit_type", 9) == 0)
			existingObj->s_unit.unit_type = property->number;
		else if (strncmp(property->key, "teamId", 6) == 0)
		{
			switch ((int)existingObj->type)
			{
				case OBJ_CORE: existingObj->s_core.team_id = property->number; break;
				case OBJ_UNIT: existingObj->s_unit.team_id = property->number; break;
			}
		}
		else if (strncmp(property->key, "gems", 7) == 0)
		{
			switch ((int)existingObj->type)
			{
				case OBJ_CORE: existingObj->s_core.gems = property->number; break;
				case OBJ_UNIT: existingObj->s_unit.gems = property->number; break;
				case OBJ_GEM_PILE:
				case OBJ_DEPOSIT: existingObj->s_resource_gems_pile.gems = property->number; break;
			}
		}
		else if (strncmp(property->key, "ActionCooldown", 12) == 0)
			existingObj->s_unit.action_cooldown = property->number;
		else if (strncmp(property->key, "countdown", 9) == 0)
			existingObj->s_bomb.countdown = property->number;
	}
}

static void core_static_applyObjToArray(json_node *new_obj)
{
	if (new_obj->type != JSON_TYPE_OBJECT)
		return; // empty obj, no updates

	long unsigned int new_obj_id = -1;
	long unsigned int new_obj_type = -1;
	long unsigned int new_obj_team_id = -1;
	long unsigned int new_obj_unit_type = -1;

	for (int i = 0; new_obj->array && new_obj->array[i]; i++)
	{
		json_node *parameter = new_obj->array[i];
		if (strncmp(parameter->key, "id", 2) == 0)
			new_obj_id = parameter->number;
		else if (strncmp(parameter->key, "teamId", 6) == 0)
			new_obj_team_id = parameter->number;
		else if (strncmp(parameter->key, "unit_type", 9) == 0)
			new_obj_unit_type = parameter->number;
		else if (strncmp(parameter->key, "type", 4) == 0)
			new_obj_type = parameter->number;
	}

	for (size_t index = 0; game.objects[index] != NULL; index++)
	{
		// Update obj -> id
		bool id_match = (game.objects[index]->id == new_obj_id);
		// Update uninitialized unit initially
		bool spawn_unit_match = (new_obj_type == OBJ_UNIT && \
				game.objects[index]->state == STATE_UNINITIALIZED && \
				game.objects[index]->s_unit.unit_type == new_obj_unit_type && \
				game.objects[index]->s_unit.team_id == new_obj_team_id);
		if (id_match || spawn_unit_match)
		{
			core_static_updateObj(game.objects[index], new_obj);
			return;
		}
	}

	if (new_obj_type == OBJ_UNIT && new_obj_team_id == game.my_team_id)
	{
		printf("Error matching team units. This is never supposed to happen. Have you freed something you shouldn't have? Never free t_obj*s, just read them. Troublemaker: [id %lu, unit_type %lu, team %lu]\n", new_obj_id, new_obj_unit_type, new_obj_team_id);
	}

	// Add new obj
	size_t arrLen = 0;
	while (game.objects[arrLen] != NULL)
		arrLen++;
	game.objects = realloc(game.objects, sizeof(t_obj *) * (arrLen + 2));
	game.objects[arrLen + 1] = NULL;
	game.objects[arrLen] = malloc(sizeof(t_obj));
	core_static_updateObj(game.objects[arrLen], new_obj);
	game.objects[arrLen]->state = STATE_ALIVE;
}

void core_internal_parse_state(char *json)
{
	json_node *root = string_to_json(json);

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

	// update action cooldowns
	for (size_t i = 0; game.objects[i]; i++)
		if (game.objects[i]->type == OBJ_UNIT)
			if (game.objects[i]->s_unit.action_cooldown > 0)
				game.objects[i]->s_unit.action_cooldown--;

	game.elapsed_ticks = (unsigned long)json_find(root, "tick")->number;

	json_node *objects = json_find(root, "objects");
	if (objects && objects->type == JSON_TYPE_ARRAY)
		for (int i = 0; objects->array != NULL && objects->array[i] != NULL; i++)
			core_static_applyObjToArray(objects->array[i]);

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

	// print errors from last tick if there were any
	json_node *errors = json_find(root, "errors");
	if (errors && errors->type == JSON_TYPE_ARRAY)
	{
		for (int i = 0; errors->array != NULL && errors->array[i] != NULL; i++)
		{
			json_node *error = errors->array[i];
			printf("\033[31m%s\033[0m\n", error->string);
		}
	}

	free_json(root);
}
