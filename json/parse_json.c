#include "parse_json.h"

t_json	ft_parse_json(char *json)
{
	jsmn_parser		parser;
	t_json			json_data;
	unsigned int	num_tokens;

	jsmn_init(&parser);
	num_tokens = 64;
	json_data.tokens = malloc(sizeof(jsmntok_t) * num_tokens);
	if (!json_data.tokens)
	{
		perror("Failed to allocate memory for tokens\n");
		exit(1);
	}
	json_data.token_len = jsmn_parse(&parser, json, strlen(json), json_data.tokens, num_tokens);
	while (json_data.token_len == JSMN_ERROR_NOMEM)
	{
		num_tokens *= 2;
		json_data.tokens = realloc(json_data.tokens, sizeof(jsmntok_t) * num_tokens);
		if (!json_data.tokens)
		{
			perror("Failed to reallocate memory for tokens\n");
			exit(1);
		}
		json_data.token_len = jsmn_parse(&parser, json, strlen(json), json_data.tokens, num_tokens);
	}

	return (json_data);
}

void	ft_parse_json_config(char *json)
{
	t_json	json_data = ft_parse_json(json);
	int	token_ind = 1;

	game.config.width = ft_find_parse_ulong("height", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.height = ft_find_parse_ulong("width", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.idle_income = ft_find_parse_ulong("idle_income", &token_ind, json_data.token_len, json_data.tokens, json);
	game.config.core_hp = ft_find_parse_ulong("core_hp", &token_ind, json_data.token_len, json_data.tokens, json);

	game.config.unit_configs = ft_parse_unit_config(token_ind, json_data.token_len, json_data.tokens, json);
	game.config.team_configs = ft_parse_team_config(token_ind, json_data.token_len, json_data.tokens, json);
}

void	ft_parse_json_state(char *json)
{
	t_json	json_data = ft_parse_json(json);
	int	token_ind = 1;

	game.status = ft_find_parse_ulong("status", &token_ind, json_data.token_len, json_data.tokens, json);

	free(game.teams);
	game.teams = ft_parse_teams(token_ind, json_data.token_len, json_data.tokens, json);

	free(game.cores);
	game.cores = ft_parse_cores(token_ind, json_data.token_len, json_data.tokens, json);

	free(game.resources);
	game.resources = ft_parse_resources(token_ind, json_data.token_len, json_data.tokens, json);

	free(game.units);
	game.units = ft_parse_units(token_ind, json_data.token_len, json_data.tokens, json);
}
