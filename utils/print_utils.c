#include "utils.h"

void	ft_print_status()
{
	const t_status	*status = &game.status;

	printf("Status: ");
	if (*status == STATUS_OK)
		printf("STATUS_OK\n");
	else if (*status == STATUS_PAUSED)
		printf("STATUS_PAUSED\n");
	else if (*status == STATUS_END)
		printf("STATUS_END\n");
	else if (*status == STATUS_WAIT_FOR_CLIENTS)
		printf("STATUS_WAIT_FOR_CLIENTS\n");
	else
		printf("UNKNOWN\n");
}

void	ft_print_teams()
{
	t_team	**teams = game.teams;

	printf("Your team id: %lu\n", game.my_team_id);
	printf("Teams:\n");
	if (teams == NULL)
		return;
	int index = 0;
	while (teams[index] != NULL)
	{
		printf("- id: %lu balance: %lu\n", teams[index]->id, teams[index]->balance);
		index++;
	}
}

void	ft_print_cores()
{
	t_obj	**cores = game.cores;

	printf("Cores:\n");
	if (cores == NULL)
		return;
	int index = 0;
	while (cores[index] != NULL)
	{
		printf("- id: %lu team_id: %lu x: %lu y: %lu hp: %lu\n", cores[index]->id, cores[index]->s_core.team_id, cores[index]->x, cores[index]->y, cores[index]->hp);
		index++;
	}
}

void	ft_print_resources()
{
	t_obj	**resources = game.resources;

	printf("Resources:\n");
	if (resources == NULL)
		return;
	int index = 0;
	while (resources[index] != NULL)
	{
		printf("- id: %lu x: %lu y: %lu hp: %lu\n", resources[index]->id, resources[index]->x, resources[index]->y, resources[index]->hp);
		index++;
	}
}

void	ft_print_units()
{
	t_obj	**units = game.units;

	printf("Units:\n");
	if (units == NULL)
		return;
	int index = 0;
	while (units[index] != NULL)
	{
		printf("- id: %lu type_id: %lu team_id: %lu x: %lu y: %lu hp: %lu\n", units[index]->id, units[index]->s_unit.type_id, units[index]->s_unit.team_id, units[index]->x, units[index]->y, units[index]->hp);
		index++;
	}
}

void ft_print_team_config(const t_team_config *team_config) {
	if (team_config == NULL || team_config->id == 0)
		return;
	printf("- ID: %lu\n", team_config->id);
	printf("  Name: %s\n", team_config->name);
}

void ft_print_unit_config(const t_unit_config *unit_config) {
	if (unit_config == NULL || unit_config->type_id == 0)
		return;
	printf("- Name: %s\n", unit_config->name);
	printf("  Type ID: %u\n", unit_config->type_id);
	printf("  Cost: %lu\n", unit_config->cost);
	printf("  HP: %lu\n", unit_config->hp);
	printf("  Damage Core: %lu\n", unit_config->dmg_core);
	printf("  Damage Unit: %lu\n", unit_config->dmg_unit);
	printf("  Damage Resource: %lu\n", unit_config->dmg_resource);
	printf("  Max Range: %lu\n", unit_config->max_range);
	printf("  Min Range: %lu\n", unit_config->min_range);
	printf("  Speed: %lu\n", unit_config->speed);
}

void ft_print_resource_config(const t_resource_config *resource_config) {
	if (resource_config == NULL || resource_config->type_id == 0)
		return;
	printf("  Type ID: %lu\n", resource_config->type_id);
	printf("  HP: %lu\n", resource_config->hp);
	printf("  Balance_Value: %lu\n", resource_config->balance_value);
}

void	ft_print_game_config()
{
	const t_config	*config = &game.config;

	if (config == NULL)
		return;
	printf("Config Height: %lu\n", config->height);
	printf("Config Width: %lu\n", config->width);
	printf("Config Idle Income: %lu\n", config->idle_income);
	printf("Config Core HP: %lu\n", config->core_hp);

	printf("Team Configs:\n");
	unsigned int ind = 0;
	if (config->teams != NULL)
	{
		while (config->teams[ind].id != 0)
			ft_print_team_config(&config->teams[ind++]);
	}

	printf("Unit Configs:\n");
	ind = 0;
	if (config->units != NULL)
	{
		while (config->units[ind].type_id != 0)
			ft_print_unit_config(&config->units[ind++]);
	}

	printf("Resource Configs:\n");
	ind = 0;
	if (config->resources != NULL)
	{
		while (config->resources[ind].type_id != 0)
			ft_print_resource_config(&config->resources[ind++]);
	}
}

void	ft_print_all()
{
	ft_print_game_config(&game.config);
	ft_print_status(game.status);
	ft_print_teams(game.teams);
	ft_print_cores(game.cores);
	ft_print_resources(game.resources);
	ft_print_units(game.units);
}
