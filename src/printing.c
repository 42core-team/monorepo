#include "con_lib.h"

void ft_print_status()
{
	const t_status *status = &game.status;

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

void ft_print_cores()
{
	t_obj **cores = game.cores;

	printf("Cores:\n");
	if (cores == NULL)
		return;
	int index = 0;
	while (cores[index] != NULL)
	{
		char *state = "ALIVE";
		if (cores[index]->state == STATE_DEAD)
			state = "DEAD";
		else if (cores[index]->state == STATE_UNINITIALIZED)
			state = "UNINITIALIZED";
		printf("- id: %lu team_id: %lu x: %u y: %u hp: %lu state: %s balance: %lu\n", cores[index]->id, cores[index]->s_core.team_id, cores[index]->pos.x, cores[index]->pos.y, cores[index]->hp, state, cores[index]->s_core.balance);
		index++;
	}
}

void ft_print_resources()
{
	t_obj **resources = game.resources;

	printf("Resources:\n");
	if (resources == NULL)
		return;
	int index = 0;
	while (resources[index] != NULL)
	{
		char *state = "ALIVE";
		if (resources[index]->state == STATE_DEAD)
			state = "DEAD";
		else if (resources[index]->state == STATE_UNINITIALIZED)
			state = "UNINITIALIZED";
		printf("- id: %lu x: %u y: %u hp: %lu state: %s balance: %lu\n", resources[index]->id, resources[index]->pos.x, resources[index]->pos.y, resources[index]->hp, state, resources[index]->s_resource.balance);
		index++;
	}
}

void ft_print_units()
{
	t_obj **units = game.units;

	printf("Units:\n");
	if (units == NULL)
		return;
	int index = 0;
	while (units[index] != NULL)
	{
		char *state = "ALIVE";
		if (units[index]->state == STATE_DEAD)
			state = "DEAD";
		else if (units[index]->state == STATE_UNINITIALIZED)
			state = "UNINITIALIZED";
		printf("- id: %lu unit_type: %lu team_id: %lu x: %u y: %u hp: %lu state: %s balance: %lu next_movement_opportunity %lu\n", units[index]->id, units[index]->s_unit.unit_type, units[index]->s_unit.team_id, units[index]->pos.x, units[index]->pos.y, units[index]->hp, state, units[index]->s_unit.balance, units[index]->s_unit.next_movement_opp);
		index++;
	}
}

void ft_print_walls()
{
	t_obj **walls = game.walls;

	printf("Walls:\n");
	if (walls == NULL)
		return;
	int index = 0;
	while (walls[index] != NULL)
	{
		char *state = "ALIVE";
		if (walls[index]->state == STATE_DEAD)
			state = "DEAD";
		else if (walls[index]->state == STATE_UNINITIALIZED)
			state = "UNINITIALIZED";
		printf("- id: %lu x: %u y: %u hp: %lu state: %s\n", walls[index]->id, walls[index]->pos.x, walls[index]->pos.y, walls[index]->hp, state);
		index++;
	}
}

void ft_print_unit_config(const t_unit_config *unit_config)
{
	if (unit_config == NULL)
		return;
	printf("- Name: %s\n", unit_config->name);
	printf("  Unit Type: %u\n", unit_config->unit_type);
	printf("  Cost: %lu\n", unit_config->cost);
	printf("  HP: %lu\n", unit_config->hp);
	printf("  Damage Core: %ld\n", unit_config->dmg_core);
	printf("  Damage Unit: %ld\n", unit_config->dmg_unit);
	printf("  Damage Resource: %ld\n", unit_config->dmg_resource);
	printf("  Damage Wall: %ld\n", unit_config->dmg_wall);
	printf("  Speed: %lu\n", unit_config->speed);
}

void ft_print_game_config()
{
	const t_config *config = &game.config;

	if (config == NULL)
		return;
	printf("Config Height: %lu\n", config->height);
	printf("Config Width: %lu\n", config->width);
	printf("Config Tick Rate: %lu\n", config->tick_rate);
	printf("Config Idle Income: %lu\n", config->idle_income);
	printf("Config Idle Income Timeout: %lu\n", config->idle_income_timeout);
	printf("Config Core HP: %lu\n", config->core_hp);
	printf("Config Initial Balance: %lu\n", config->initial_balance);
	printf("Config Resource HP: %lu\n", config->resource_hp);
	printf("Config Resource Income: %lu\n", config->resource_income);

	printf("Unit Configs:\n");
	int ind = 0;
	if (config->units != NULL)
		while (config->units[ind] != NULL)
			ft_print_unit_config(config->units[ind++]);
}

void ft_print_all()
{
	ft_print_game_config(&game.config);
	ft_print_status(game.status);
	ft_print_cores(game.cores);
	ft_print_resources(game.resources);
	ft_print_walls(game.walls);
	ft_print_units(game.units);
}
