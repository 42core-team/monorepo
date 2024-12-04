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
			*((*arr)[index]) = obj;
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
		bool matches = false;
		matches = (*arr)[index]->id == 0;
		if ((*arr) == game.units && (*arr)[index]->s_unit.type_id != obj.s_unit.type_id)
			matches = false;

		if (matches)
		{
			*((*arr)[index]) = obj;
			objInserted = true;
			break;
		}
		index++;
	}
	if (objInserted)
		return ;

	if ((*arr) == game.units)
	{
		printf("Error matching units. Troublemaker: %lu, %lu\n", obj.id, obj.s_unit.type_id);
	}

	// 3. Add to the back
	size_t arrLen = 0;
	while ((*arr)[arrLen] != NULL)
		arrLen++;
	(*arr) = realloc((*arr), sizeof(t_obj *) * (arrLen + 2));
	(*arr)[arrLen + 1] = NULL;
	(*arr)[arrLen] = malloc(sizeof(t_obj));
	*((*arr)[arrLen]) = obj;
}

void ft_parse_cores(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int	index, last_json_index, next_token_ind;

	token_ind = ft_find_token_one("cores", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return ;
	last_json_index = tokens[token_ind].end;

	if (game.cores == NULL)
	{
		game.cores = malloc(sizeof(t_obj *) * 1);
		game.cores[0] = NULL;
	}

	for (size_t i = 0; game.cores[i] != NULL; i++)
		if (game.cores[i]->state == STATE_ALIVE)
			game.cores[i]->state = STATE_DEAD;

	index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		t_obj readCore;
		readCore.type = OBJ_CORE;
		readCore.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readCore.s_core.team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		readCore.x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		readCore.y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		readCore.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		apply_obj_to_arr(readCore, &game.cores);

		index++;
	}
}

void	ft_parse_resources(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int	index, last_json_index, next_token_ind;

	token_ind = ft_find_token_one("resources", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return ;
	last_json_index = tokens[token_ind].end;

	if (game.resources == NULL)
	{
		game.resources = malloc(sizeof(t_obj *) * 1);
		game.resources[0] = NULL;
	}

	for (size_t i = 0; game.resources[i] != NULL; i++)
		if (game.resources[i]->state == STATE_ALIVE)
			game.resources[i]->state = STATE_DEAD;

	index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		t_obj readResource;
		readResource.type = OBJ_RESOURCE;
		readResource.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readResource.x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		readResource.y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		readResource.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		apply_obj_to_arr(readResource, &game.resources);

		index++;
	}
}

void	ft_parse_units(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int	index, last_json_index;

	token_ind = ft_find_token_one("units", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return ;
	last_json_index = tokens[token_ind].end;

	if (game.units == NULL)
	{
		game.units = malloc(sizeof(t_obj *) * 1);
		game.units[0] = NULL;
	}

	for (size_t i = 0; game.units[i] != NULL; i++)
		if (game.units[i]->state == STATE_ALIVE)
			game.units[i]->state = STATE_DEAD;

	index = 0;
	while (token_ind != -1 && tokens[ft_find_token_one("id", token_ind, token_len, tokens, json)].end <= last_json_index)
	{
		t_obj readUnit;
		readUnit.type = OBJ_UNIT;
		readUnit.id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		readUnit.s_unit.type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		readUnit.s_unit.team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		readUnit.hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);
		readUnit.x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		readUnit.y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);

		apply_obj_to_arr(readUnit, &game.units);

		index++;
	}
}

#define TEAM_COUNT 2
void ft_parse_teams(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int		last_json_index, next_token_ind;

	token_ind = ft_find_token_one("teams", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return ;
	last_json_index = tokens[token_ind].end;

	if (game.teams == NULL)
	{
		game.teams = malloc(sizeof(t_team *) * (TEAM_COUNT + 1));
		game.teams[TEAM_COUNT] = NULL;

		for (size_t i = 0; i < TEAM_COUNT; i++)
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

		game.teams[index]->id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		game.teams[index]->balance = ft_find_parse_ulong("balance", &token_ind, token_len, tokens, json);

		index++;
	}
}
