#include "parse_json.h"

t_team_config	*ft_parse_team_config(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_team_config	*teams;
	int				last_json_index, next_token_ind;

	token_ind = ft_find_token_one("teams", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return (NULL);
	last_json_index = tokens[token_ind].end;

	teams = malloc(sizeof(t_team_config));
	teams[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		teams = realloc(teams, sizeof(t_team_config) * (index + 2));
		teams[index + 1].id = 0;

		teams[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		teams[index].name = ft_find_parse_str("name", &token_ind, token_len, tokens, json);

		index++;
	}

	return (teams);
}

t_unit_config	*ft_parse_unit_config(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_unit_config	*units;
	int				last_json_index, next_token_ind;

	token_ind = ft_find_token_one("units", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return (NULL);
	last_json_index = tokens[token_ind].end;

	units = malloc(sizeof(t_unit_config));
	units[0].type_id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("name", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		units = realloc(units, sizeof(t_unit_config) * (index + 2));
		units[index + 1].type_id = 0;

		units[index].name = ft_find_parse_str("name", &token_ind, token_len, tokens, json);
		units[index].type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		units[index].cost = ft_find_parse_ulong("cost", &token_ind, token_len, tokens, json);
		units[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);
		units[index].dmg_core = ft_find_parse_long("dmg_core", &token_ind, token_len, tokens, json);
		units[index].dmg_unit = ft_find_parse_long("dmg_unit", &token_ind, token_len, tokens, json);
		units[index].dmg_resource = ft_find_parse_long("dmg_resource", &token_ind, token_len, tokens, json);
		units[index].max_range = ft_find_parse_ulong("max_range", &token_ind, token_len, tokens, json);
		units[index].min_range = ft_find_parse_ulong("min_range", &token_ind, token_len, tokens, json);
		units[index].speed = ft_find_parse_ulong("speed", &token_ind, token_len, tokens, json);

		index++;
	}

	return (units);
}

t_resource_config	*ft_parse_resource_config(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_resource_config	*resource_configs;
	int					last_json_index, next_token_ind;

	token_ind = ft_find_token_one("resources", token_ind, token_len, tokens, json);
	if (token_ind == -1)
		return (NULL);
	last_json_index = tokens[token_ind].end;

	resource_configs = malloc(sizeof(t_resource_config));
	resource_configs[0].type_id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		next_token_ind = ft_find_token_one("type_id", token_ind, token_len, tokens, json);
		if (next_token_ind == -1 || tokens[next_token_ind].end > last_json_index)
			break;

		resource_configs = realloc(resource_configs, sizeof(t_resource_config) * (index + 2));
		resource_configs[index + 1].type_id = 0;

		resource_configs[index].type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		resource_configs[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);
		resource_configs[index].balance_value = ft_find_parse_ulong("balance_value", &token_ind, token_len, tokens, json);

		index++;
	}

	return (resource_configs);
}
