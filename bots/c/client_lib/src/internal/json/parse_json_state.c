#include "core_lib_internal.h"

static bool core_static_objDiesInDiff(json_node *updates)
{
	if (!updates || updates->type != JSON_TYPE_OBJECT) return false;

	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *field = updates->array[i];
		if (field->key && strcmp(field->key, "state") == 0)
			return (field->string && strcmp(field->string, "dead") == 0);
	}
	return false;
}

static unsigned long core_static_extract_id(json_node *obj)
{
	if (!obj || obj->type != JSON_TYPE_OBJECT) return ULONG_MAX;
	for (int i = 0; obj->array && obj->array[i]; i++)
	{
		json_node *p = obj->array[i];
		if (p->key && strcmp(p->key, "id") == 0) return (unsigned long)p->number;
	}
	return ULONG_MAX;
}

static void core_static_removeObjWithId(unsigned long id)
{
	for (size_t i = 0; game.objects[i]; i++)
	{
		if (game.objects[i]->id == id)
		{
			core_internal_freeObject(game.objects[i]);
			for (size_t j = i; game.objects[j]; j++)
				game.objects[j] = game.objects[j + 1];
			return;
		}
	}
}

static void core_static_parseComponents(t_obj *obj, json_node *field)
{
	if (!obj || obj->type != OBJ_UNIT) return;
	if (!field || field->type != JSON_TYPE_ARRAY) return;

	if (obj->s_unit.components)
	{
		for (size_t i = 0; obj->s_unit.components[i]; i++)
			free(obj->s_unit.components[i]);

		free(obj->s_unit.components);
		obj->s_unit.components = NULL;
	}

	size_t count = 0;
	while (field->array && field->array[count])
		count++;

	char **components = malloc(sizeof(char *) * (count + 1));
	if (!components)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < count; i++)
	{
		json_node *component = field->array[i];

		if (component->type == JSON_TYPE_STRING && component->string)
			components[i] = strdup(component->string);
		else
			components[i] = strdup("");

		if (!components[i])
		{
			for (size_t j = 0; j < i; j++)
				free(components[j]);

			free(components);
			perror("strdup");
			exit(EXIT_FAILURE);
		}
	}

	components[count] = NULL;
	obj->s_unit.components = components;
}
static void core_static_parseProperties(t_obj *obj, json_node *field)
{
	if (!obj || obj->type != OBJ_UNIT) return;
	if (!field || field->type != JSON_TYPE_OBJECT) return;

	for (int i = 0; field->array && field->array[i]; i++)
	{
		json_node *prop = field->array[i];

		if (prop->key && prop->type == JSON_TYPE_NUMBER)
		{
			if (strcmp(prop->key, "hp") == 0)
				obj->s_unit.properties.hp = prop->number;
			else if (strcmp(prop->key, "baseActionCooldown") == 0)
				obj->s_unit.properties.base_action_cooldown = prop->number;
			else if (strcmp(prop->key, "balancePerCooldownStep") == 0)
				obj->s_unit.properties.balance_per_cooldown_step = prop->number;
			else if (strcmp(prop->key, "maxBalance") == 0)
				obj->s_unit.properties.max_balance = prop->number;
			else if (strcmp(prop->key, "damageReductionPercent") == 0)
				obj->s_unit.properties.damage_reduction_percent = prop->number;
			else if (strcmp(prop->key, "damageCore") == 0)
				obj->s_unit.properties.damage_core = prop->number;
			else if (strcmp(prop->key, "damageUnit") == 0)
				obj->s_unit.properties.damage_unit = prop->number;
			else if (strcmp(prop->key, "damageObject") == 0)
				obj->s_unit.properties.damage_object = prop->number;
			else if (strcmp(prop->key, "postSpawnCoreCooldown") == 0)
				obj->s_unit.properties.post_spawn_core_cooldown = prop->number;
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

	// parse object type first so later parsed conditional properties won't be skipped if json is out of order
	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *field = updates->array[i];
		if (field->key && strcmp(field->key, "type") == 0)
		{
			existingObj->type = field->number;
			break;
		}
	}

	for (int i = 0; updates->array && updates->array[i]; i++)
	{
		json_node *field = updates->array[i];

		// even non-changing properties are included so the function can also fully set all fields of an empty obj
		if (strcmp(field->key, "x") == 0)
			existingObj->pos.x = field->number;
		else if (strcmp(field->key, "y") == 0)
			existingObj->pos.y = field->number;
		else if (strcmp(field->key, "hp") == 0)
			existingObj->hp = field->number;
		else if (strcmp(field->key, "type") == 0)
			existingObj->type = field->number;
		else if (strcmp(field->key, "id") == 0)
			existingObj->id = field->number;
		else if (strcmp(field->key, "name") == 0)
		{
			if (existingObj->type == OBJ_UNIT && field->type == JSON_TYPE_STRING && field->string)
			{
				free(existingObj->s_unit.name);
				existingObj->s_unit.name = strdup(field->string);
			}
		}
		else if (strcmp(field->key, "components") == 0)
			core_static_parseComponents(existingObj, field);
		else if (strcmp(field->key, "properties") == 0)
			core_static_parseProperties(existingObj, field);
		else if (strcmp(field->key, "teamId") == 0)
		{
			switch ((int)existingObj->type)
			{
			case OBJ_CORE:
				existingObj->s_core.team_id = field->number;
				break;
			case OBJ_UNIT:
				existingObj->s_unit.team_id = field->number;
				break;
			}
		}
		else if (strcmp(field->key, "gems") == 0)
		{
			switch ((int)existingObj->type)
			{
			case OBJ_CORE:
				existingObj->s_core.gems = field->number;
				break;
			case OBJ_UNIT:
				existingObj->s_unit.gems = field->number;
				break;
			case OBJ_GEM_PILE:
			case OBJ_DEPOSIT:
				existingObj->s_deposit_gems_pile.gems = field->number;
				break;
			}
		}
		else if (strcmp(field->key, "ActionCooldown") == 0)
			existingObj->s_unit.action_cooldown = field->number;
		else if (strcmp(field->key, "SpawnCooldown") == 0)
			existingObj->s_core.spawn_cooldown = field->number;
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
			game.objects[i]->s_unit.action_cooldown--;
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
