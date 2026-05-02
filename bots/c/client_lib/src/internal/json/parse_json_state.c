#include "core_lib_internal.h"

static bool core_static_objDiesInDiff(json_node *updates)
{
	if (!updates || updates->type != JSON_TYPE_OBJECT) return false;

	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *property = updates->array[i];
		if (property->key && strncmp(property->key, "state", 5) == 0)
			return (property->string && strncmp(property->string, "dead", 4) == 0);
	}
	return false;
}

static unsigned long core_static_extract_id(json_node *obj)
{
	if (!obj || obj->type != JSON_TYPE_OBJECT) return ULONG_MAX;
	for (int i = 0; obj->array && obj->array[i]; i++)
	{
		json_node *p = obj->array[i];
		if (p->key && strncmp(p->key, "id", 2) == 0) return (unsigned long)p->number;
	}
	return ULONG_MAX;
}

static void core_static_removeObjWithId(unsigned long id)
{
	for (size_t i = 0; game.objects[i]; i++)
	{
		if (game.objects[i]->id == id)
		{
			free(game.objects[i]);
			for (size_t j = i; game.objects[j]; j++)
				game.objects[j] = game.objects[j + 1];
			return;
		}
	}
}

static void core_static_updateObj(t_obj *existingObj, json_node *updates)
{
	if (updates->type != JSON_TYPE_OBJECT)
	{
		char *updatesFormatted = json_to_string(updates);
		printf("Error: Supplied json node is not an array. Got: %s\n", updatesFormatted);
		free(updatesFormatted);
		return;
	}

	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *property = updates->array[i];

		// even non-changing properties are included so the function can also fully set all fields of an empty obj
		if (strncmp(property->key, "x", 1) == 0)
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
			case OBJ_CORE:
				existingObj->s_core.team_id = property->number;
				break;
			case OBJ_UNIT:
				existingObj->s_unit.team_id = property->number;
				break;
			}
		}
		else if (strncmp(property->key, "gems", 7) == 0)
		{
			switch ((int)existingObj->type)
			{
			case OBJ_CORE:
				existingObj->s_core.gems = property->number;
				break;
			case OBJ_UNIT:
				existingObj->s_unit.gems = property->number;
				break;
			case OBJ_GEM_PILE:
			case OBJ_DEPOSIT:
				existingObj->s_deposit_gems_pile.gems = property->number;
				break;
			}
		}
		else if (strncmp(property->key, "ActionCooldown", 12) == 0)
			existingObj->s_unit.action_cooldown = property->number;
		else if (strncmp(property->key, "SpawnCooldown", 11) == 0)
			existingObj->s_core.spawn_cooldown = property->number;
	}
}

static void core_static_applyObjToArray(json_node *new_obj)
{
	if (new_obj->type != JSON_TYPE_OBJECT) return; // empty obj, no updates

	// Update existing obj
	for (size_t index = 0; game.objects[index] != NULL; index++)
	{
		if (game.objects[index]->id == core_static_extract_id(new_obj))
		{
			core_static_updateObj(game.objects[index], new_obj);
			return;
		}
	}

	// Add new obj
	size_t arrLen = 0;
	while (game.objects[arrLen] != NULL)
		arrLen++;
	game.objects = realloc(game.objects, sizeof(t_obj *) * (arrLen + 2));
	game.objects[arrLen + 1] = NULL;
	game.objects[arrLen] = malloc(sizeof(t_obj));
	memset(game.objects[arrLen], 0, sizeof(t_obj));
	core_static_updateObj(game.objects[arrLen], new_obj);
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

	game.elapsed_ticks = (unsigned long)json_find(root, "tick")->number;

	json_node *objects = json_find(root, "objects");
	if (objects && objects->type == JSON_TYPE_ARRAY)
	{
		for (int i = 0; objects->array && objects->array[i]; i++)
		{
			json_node *diff = objects->array[i];

			if (core_static_objDiesInDiff(diff))
			{
				unsigned long dead_id = core_static_extract_id(diff);
				if (dead_id != ULONG_MAX) core_static_removeObjWithId(dead_id);
				continue;
			}
			core_static_applyObjToArray(diff);
		}
	}

	// update action cooldowns & spawn cooldowns
	for (size_t i = 0; game.objects[i]; i++)
	{
		if (game.objects[i]->type == OBJ_UNIT)
		{
			if (game.objects[i]->s_unit.action_cooldown > 0) game.objects[i]->s_unit.action_cooldown--;
		}
		else if (game.objects[i]->type == OBJ_CORE)
		{
			if (game.objects[i]->s_core.spawn_cooldown > 0) game.objects[i]->s_core.spawn_cooldown--;
		}
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
