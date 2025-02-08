#include "parse_json.h"

static t_json	ft_parse_json(char *json)
{
	jsmn_parser		parser;
	t_json			json_data;
	unsigned int	num_tokens;

	jsmn_init(&parser);
	num_tokens = 64;
	json_data.tokens = malloc(sizeof(jsmntok_t) * num_tokens);
	if (!json_data.tokens)
		ft_perror_exit("Failed to allocate memory for tokens\n");
	json_data.token_len = jsmn_parse(&parser, json, strlen(json), json_data.tokens, num_tokens);
	while (json_data.token_len == JSMN_ERROR_NOMEM)
	{
		num_tokens *= 2;
		json_data.tokens = realloc(json_data.tokens, sizeof(jsmntok_t) * num_tokens);
		if (!json_data.tokens)
			ft_perror_exit("Failed to reallocate memory for tokens\n");
		json_data.token_len = jsmn_parse(&parser, json, strlen(json), json_data.tokens, num_tokens);
	}

	return (json_data);
}

t_unit_config	*ft_parse_unit_config(int *token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	int units_index = ft_find_token_one("units", *token_ind, token_len, tokens, json);
	if (units_index == -1)
		return NULL;
	
	int array_size = tokens[units_index].size;
	t_unit_config *units = malloc(sizeof(t_unit_config) * (array_size + 1));
	if (!units)
		ft_perror_exit("Failed to allocate memory for unit configs\n");
	
	int i;
	int index = units_index + 1;
	for (i = 0; i < array_size; i++)
	{
		units[i].name = ft_find_parse_str("name", &index, token_len, tokens, json);
		units[i].cost = ft_find_parse_ulong("cost", &index, token_len, tokens, json);
		units[i].hp = ft_find_parse_ulong("hp", &index, token_len, tokens, json);
		units[i].speed = ft_find_parse_ulong("speed", &index, token_len, tokens, json);
		units[i].dmg_core = ft_find_parse_long("damageCore", &index, token_len, tokens, json);
		units[i].dmg_unit = ft_find_parse_long("damageUnit", &index, token_len, tokens, json);
		units[i].dmg_resource = ft_find_parse_long("damageResource", &index, token_len, tokens, json);
		units[i].dmg_wall = ft_find_parse_long("damageWall", &index, token_len, tokens, json);
	}
	units[array_size].name = NULL;
	
	return units;
}

void	ft_parse_json_config(char *json)
{
	t_json	json_data = ft_parse_json(json);
	int	token_ind = 0;

	game.config.width = ft_find_parse_ulong("width", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.height = ft_find_parse_ulong("height", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.tick_rate = ft_find_parse_ulong("tickRate", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.idle_income = ft_find_parse_ulong("idleIncome", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.idle_income_timeout = ft_find_parse_ulong("idleIncomeTimeOut", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.resource_hp = ft_find_parse_ulong("resourceHp", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.resource_income = ft_find_parse_ulong("resourceIncome", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.core_hp = ft_find_parse_ulong("coreHp", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.initial_balance = ft_find_parse_ulong("initialBalance", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.wall_hp = ft_find_parse_ulong("wallHp", &token_ind, json_data.token_len, json_data.tokens, json);
	
	game.config.units = ft_parse_unit_config(&token_ind, json_data.token_len, json_data.tokens, json);
	
	free(json_data.tokens);
}

void	ft_parse_json_state(char *json)
{
	t_json	json_data = ft_parse_json(json);
	int	token_ind = 1;

	// ft_free_game(); // Risky business is afoot

	game.status = ft_find_parse_ulong("status", &token_ind, json_data.token_len, json_data.tokens, json);
	game.elapsed_ticks = ft_find_parse_ulong("elapsed_ticks", &token_ind, json_data.token_len, json_data.tokens, json);
	ft_parse_cores(token_ind, json_data.token_len, json_data.tokens, json);
	ft_parse_resources(token_ind, json_data.token_len, json_data.tokens, json);
	ft_parse_units(token_ind, json_data.token_len, json_data.tokens, json);
	ft_parse_walls(token_ind, json_data.token_len, json_data.tokens, json);

	free(json_data.tokens);
}
