#include "parse_json.h"

void ft_parse_cores(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int	index = 0;

	token_ind = ft_find_token_one("cores", token_ind, token_len, tokens, json);
	if (token_ind == -1)
	{
		for (int i = 0; game.cores && game.cores[i]; i++)
			game.cores[i]->state = STATE_DEAD;
		return ;
	}
	int cores_count = tokens[token_ind].size;

	if (game.cores == NULL)
	{
		game.cores = malloc(sizeof(t_obj *) * 1);
		game.cores[0] = NULL;
	}

	for (int i = 0; game.cores[i] != NULL; i++)
		game.cores[i]->state = STATE_DEAD;

	token_ind++;
	while (index < cores_count && token_ind < token_len)
	{
		if (tokens[token_ind].type != JSMN_OBJECT)
		{
			token_ind++;
			continue;
		}

		t_obj readCore;
		readCore.type = OBJ_CORE;
		readCore.state = STATE_ALIVE;
		readCore.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readCore.s_core.team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		int pos_token_ind = ft_find_token_one("pos", token_ind, token_len, tokens, json);
		if (pos_token_ind != -1)
		{
			readCore.x = ft_find_parse_ulong("x", &pos_token_ind, token_len, tokens, json);
			readCore.y = ft_find_parse_ulong("y", &pos_token_ind, token_len, tokens, json);
		}
		readCore.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		bool readCoreInserted = false;

		// 1. LOOP: Id Matching
		int core_index = 0;
		while (game.cores[core_index] != NULL)
		{
			if (game.cores[core_index]->id == readCore.id)
			{
				*(game.cores[core_index]) = readCore
				readCoreInserted = true;
				break;
			}
			core_index++;
		}
		if (readCoreInserted)
		{
			index++;
			continue;
		}

		// 2. LOOP: Placeholder matching
		core_index = 0;
		while (game.cores[core_index] != NULL)
		{
			if (game.cores[core_index]->id == 0)
			{
				*(game.cores[core_index]) = readCore
				readCoreInserted = true;
				break;
			}
			core_index++;
		}
		if (readCoreInserted)
		{
			index++;
			continue;
		}

		// 3. New core
		int coreArrLen = 0;
		while (game.cores[coreArrLen] != NULL)
			coreArrLen++;
		game.cores = realloc(game.cores, sizeof(t_obj *) * (coreArrLen + 2));
		game.cores[coreArrLen] = malloc(sizeof(t_obj));
		game.cores[coreArrLen + 1] = NULL;
		*(game.cores[coreArrLen]) = readCore;
		game.cores[coreArrLen]->state = STATE_ALIVE;

		index++;
	}
}

#define TEAM_COUNT 2
void ft_parse_teams(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int		last_json_index, next_token_ind;

	token_ind = ft_find_token_one("teams", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return (NULL);
	last_json_index = tokens[token_ind].end;

	if (game.teams == NULL)
	{
		game.teams = malloc(sizeof(t_team *) * (TEAM_COUNT + 1));
		game.teams[TEAM_COUNT] = NULL;

		for (int i = 0; i < TEAM_COUNT; i++)
		{
			game.teams[i] = malloc(sizeof(t_team));
			game.teams[i]->id = 0;
			game.teams[i]->balance = 0;
		}
	}

	int index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		teams[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		teams[index].balance = ft_find_parse_ulong("balance", &token_ind, token_len, tokens, json);

		index++;
	}
}

void	ft_parse_resources(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int		index = 0;

	token_ind = ft_find_token_one("resources", token_ind, token_len, tokens, json);
	if (token_ind == -1)
	{
		for (int i = 0; game.resources && game.resources[i]; i++)
			game.resources[i]->state = STATE_DEAD;
		return ;
	}
	int resource_count = tokens[token_ind].size;

	if (game.resources == NULL)
	{
		game.resources = malloc(sizeof(t_obj *) * 1);
		game.cores[0] = NULL;
	}

	for (int i = 0; game.resources[i] != NULL; i++)
		game.resources[i]->state = STATE_DEAD;

	token_ind++;
	while (index < resource_count && token_ind < token_len)
	{
		if (tokens[token_ind].type != JSMN_OBJECT)
		{
			token_ind++;
			continue;
		}

		t_obj readResource;
		readResource.type = OBJ_RESOURCE;
		readResource.state = STATE_ALIVE;
		readResource.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readResource.x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		readResource.y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		readResource.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		bool readResInserted = false;

		// 1. LOOP: Id Matching
		int resource_index = 0;
		while (game.resources[resource_index] != NULL)
		{
			if (game.resources[resource_index]->id == readCore.id)
			{
				*(game.resources[resource_index]) = readCore
				readResInserted = true;
				break;
			}
			resource_index++;
		}
		if (readResInserted)
		{
			index++;
			continue;
		}

		// 2. LOOP: Placeholder matching
		resource_index = 0;
		while (game.resources[resource_index] != NULL)
		{
			if (game.resources[resource_index]->id == 0)
			{
				*(game.resources[resource_index]) = readCore
				readResInserted = true;
				break;
			}
			resource_index++;
		}
		if (readResInserted)
		{
			index++;
			continue;
		}

		// 3. New core
		int coreArrLen = 0;
		while (game.resources[coreArrLen] != NULL)
			coreArrLen++;
		game.resources = realloc(game.resources, sizeof(t_obj *) * (coreArrLen + 2));
		game.resources[coreArrLen] = malloc(sizeof(t_obj));
		game.resources[coreArrLen + 1] = NULL;
		*(game.resources[coreArrLen]) = readCore;
		game.resources[coreArrLen]->state = STATE_ALIVE;

		index++;
	}

	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		resources = realloc(resources, sizeof(t_obj) * (index + 2));
		resources[index + 1].id = 0;

		resources[index].type = OBJ_RESOURCE;
		resources[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		resources[index].x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		resources[index].y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		resources[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		index++;
	}

	return (resources);
}

void	ft_parse_units(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int		index;

	token_ind = ft_find_token_one("units", token_ind, token_len, tokens, json);
	if (token_ind == -1)
	{
		for (int i = 0; game.units && game.units[i]; i++)
			game.units[i]->state = STATE_DEAD;
		return ;
	}
	int unit_count = tokens[token_ind].size;

	if (game.units == NULL)
	{
		game.units = malloc(sizeof(t_obj *) * 1);
		game.units[0] = NULL;
	}

	for (int i = 0; game.units[i] != NULL; i++)
		game.units[i]->state = STATE_DEAD;

	token_ind++;
	while (index < unit_count && token_ind < token_len)
	{
		if (tokens[token_ind].type != JSMN_OBJECT)
		{
			token_ind++;
			continue;
		}

		t_obj readUnit;
		readUnit.type = OBJ_CORE;
		readUnit.state = STATE_ALIVE;
		readUnit.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readUnit.s_unit.team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		readUnit.s_unit.type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		int pos_token_ind = ft_find_token_one("pos", token_ind, token_len, tokens, json);
		if (pos_token_ind != -1)
		{
			readUnit.x = ft_find_parse_ulong("x", &pos_token_ind, token_len, tokens, json);
			readUnit.y = ft_find_parse_ulong("y", &pos_token_ind, token_len, tokens, json);
		}
		readUnit.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		bool readUnitInserted = false;

		// 1. LOOP: Id Matching
		int unit_index = 0;
		while (game.units[unit_index] != NULL)
		{
			if (game.units[unit_index]->id == readCore.id)
			{
				*(game.units[unit_index]) = readCore
				readUnitInserted = true;
				break;
			}
			unit_index++;
		}
		if (readUnitInserted)
		{
			index++;
			continue;
		}

		// 2. LOOP: Placeholder matching
		unit_index = 0;
		while (game.units[unit_index] != NULL)
		{
			if (game.units[unit_index]->id == 0)
			{
				*(game.units[unit_index]) = readCore
				readUnitInserted = true;
				break;
			}
			unit_index++;
		}
		if (readUnitInserted)
		{
			index++;
			continue;
		}

		// 3. New core
		int coreArrLen = 0;
		while (game.units[coreArrLen] != NULL)
			coreArrLen++;
		game.units = realloc(game.units, sizeof(t_obj *) * (coreArrLen + 2));
		game.units[coreArrLen] = malloc(sizeof(t_obj));
		game.units[coreArrLen + 1] = NULL;
		*(game.units[coreArrLen]) = readCore;
		game.units[coreArrLen]->state = STATE_ALIVE;

		index++;
	}
}
