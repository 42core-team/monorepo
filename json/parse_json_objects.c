#include "parse_json.h"

t_core	*ft_parse_cores(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_core	*cores;

	token_ind = ft_find_token_one("cores", token_ind, token_len, tokens, json);

	cores = malloc(sizeof(t_core));
	cores[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		cores = realloc(cores, sizeof(t_core) * (index + 2));
		cores[index + 1].id = 0;

		cores[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		cores[index].team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		cores[index].x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		cores[index].y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		cores[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		index++;
	}

	return (cores);
}

t_team	*ft_parse_teams(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_team	*teams;

	token_ind = ft_find_token_one("teams", token_ind, token_len, tokens, json);

	teams = malloc(sizeof(t_team));
	teams[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		teams = realloc(teams, sizeof(t_team) * (index + 2));
		teams[index + 1].id = 0;

		teams[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		teams[index].balance = ft_find_parse_ulong("balance", &token_ind, token_len, tokens, json);

		index++;
	}

	return (teams);
}

t_resource	*ft_parse_resources(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_resource	*resources;

	token_ind = ft_find_token_one("resources", token_ind, token_len, tokens, json);

	resources = malloc(sizeof(t_resource));
	resources[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		resources = realloc(resources, sizeof(t_resource) * (index + 2));
		resources[index + 1].id = 0;

		resources[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		resources[index].value = ft_find_parse_ulong("value", &token_ind, token_len, tokens, json);
		resources[index].x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		resources[index].y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		resources[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		index++;
	}

	return (resources);
}

t_unit	*ft_parse_units(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_unit	*units;

	token_ind = ft_find_token_one("units", token_ind, token_len, tokens, json);

	units = malloc(sizeof(t_unit));
	units[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		units = realloc(units, sizeof(t_unit) * (index + 2));
		units[index + 1].id = 0;

		units[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		units[index].type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		units[index].team_id = ft_find_parse_ulong("team_id", &token_ind, token_len, tokens, json);
		units[index].x = ft_find_parse_ulong("x", &token_ind, token_len, tokens, json);
		units[index].y = ft_find_parse_ulong("y", &token_ind, token_len, tokens, json);
		units[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);

		index++;
	}

	return (units);
}

t_team_config	*ft_parse_team_config(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_team_config	*team_configs;

	token_ind = ft_find_token_one("teams", token_ind, token_len, tokens, json);

	team_configs = malloc(sizeof(t_team_config));
	team_configs[0].id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		team_configs = realloc(team_configs, sizeof(t_team_config) * (index + 2));
		team_configs[index + 1].id = 0;

		team_configs[index].id = ft_find_parse_ulong("id", &token_ind, token_len, tokens, json);
		team_configs[index].name = ft_find_parse_str("name", &token_ind, token_len, tokens, json);

		index++;
	}

	return (team_configs);
}

t_unit_config	*ft_parse_unit_config(int token_ind, int token_len, jsmntok_t *tokens, char *json)
{
	t_unit_config	*unit_configs;

	token_ind = ft_find_token_one("units", token_ind, token_len, tokens, json);

	unit_configs = malloc(sizeof(t_unit_config));
	unit_configs[0].type_id = 0;

	int index = 0;
	while (token_ind != -1)
	{
		unit_configs = realloc(unit_configs, sizeof(t_unit_config) * (index + 2));
		unit_configs[index + 1].type_id = 0;

		unit_configs[index].name = ft_find_parse_str("name", &token_ind, token_len, tokens, json);
		unit_configs[index].type_id = ft_find_parse_ulong("type_id", &token_ind, token_len, tokens, json);
		unit_configs[index].cost = ft_find_parse_ulong("cost", &token_ind, token_len, tokens, json);
		unit_configs[index].hp = ft_find_parse_ulong("hp", &token_ind, token_len, tokens, json);
		unit_configs[index].dmg_core = ft_find_parse_ulong("dmg_core", &token_ind, token_len, tokens, json);
		unit_configs[index].dmg_unit = ft_find_parse_ulong("dmg_unit", &token_ind, token_len, tokens, json);
		unit_configs[index].dmg_resource = ft_find_parse_ulong("dmg_resource", &token_ind, token_len, tokens, json);
		unit_configs[index].max_range = ft_find_parse_ulong("max_range", &token_ind, token_len, tokens, json);
		unit_configs[index].min_range = ft_find_parse_ulong("min_range", &token_ind, token_len, tokens, json);
		unit_configs[index].speed = ft_find_parse_ulong("speed", &token_ind, token_len, tokens, json);

		index++;
	}

	return (unit_configs);
}
