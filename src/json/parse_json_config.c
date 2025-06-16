#include "parse_json.h"
#include "string.h"

static t_unit_config **ft_parse_unit_config(json_node *root)
{
	json_node *json = json_find(root, "units");

	int array_size = 0;
	for (int i = 0; json->array && json->array[i]; i++)
		array_size++;

	t_unit_config **units = malloc(sizeof(t_unit_config *) * (array_size + 1));
	if (!units)
		ft_perror_exit("Failed to allocate memory for unit configs\n");

	for (int i = 0; i < array_size; i++)
	{
		units[i] = malloc(sizeof(t_unit_config));
		if (!units[i])
			ft_perror_exit("Failed to allocate memory for unit config\n");

		json_node *unit_node = json->array[i];

		units[i]->name = strdup(json_find(unit_node, "name")->string);
		units[i]->unit_type = i;
		units[i]->cost = (unsigned long)json_find(unit_node, "cost")->number;
		units[i]->hp = (unsigned long)json_find(unit_node, "hp")->number;
		units[i]->speed = (unsigned long)json_find(unit_node, "speed")->number;
		units[i]->min_speed = (unsigned long)json_find(unit_node, "minSpeed")->number;
		units[i]->dmg_core = (long)json_find(unit_node, "damageCore")->number;
		units[i]->dmg_unit = (long)json_find(unit_node, "damageUnit")->number;
		units[i]->dmg_resource = (long)json_find(unit_node, "damageResource")->number;
		units[i]->dmg_wall = (long)json_find(unit_node, "damageWall")->number;
		units[i]->dmg_bomb = (unsigned long)json_find(unit_node, "damageBomb")->number;
		units[i]->attack_type = (t_attack_type)json_find(unit_node, "attackType")->number;
		units[i]->can_build = json_find(unit_node, "canBuild")->number;
	}
	units[array_size] = NULL;

	return units;
}

void ft_parse_json_config(char *json)
{
	json_node *root = string_to_json(json);

	game.config.width = (unsigned long)json_find(root, "width")->number;
	game.config.height = (unsigned long)json_find(root, "height")->number;
	game.config.tick_rate = (unsigned long)json_find(root, "tickRate")->number;
	game.config.idle_income = (unsigned long)json_find(root, "idleIncome")->number;
	game.config.idle_income_timeout = (unsigned long)json_find(root, "idleIncomeTimeOut")->number;
	game.config.resource_hp = (unsigned long)json_find(root, "resourceHp")->number;
	game.config.resource_income = (unsigned long)json_find(root, "resourceIncome")->number;
	game.config.core_hp = (unsigned long)json_find(root, "coreHp")->number;
	game.config.initial_balance = (unsigned long)json_find(root, "initialBalance")->number;
	game.config.wall_hp = (unsigned long)json_find(root, "wallHp")->number;
	game.config.wall_build_cost = (unsigned long)json_find(root, "wallBuildCost")->number;
	game.config.bomb_hp = (unsigned long)json_find(root, "bombHp")->number;
	game.config.bomb_countdown = (unsigned long)json_find(root, "bombCountdown")->number;
	game.config.bomb_throw_cost = (unsigned long)json_find(root, "bombThrowCost")->number;
	game.config.bomb_reach = (unsigned long)json_find(root, "bombReach")->number;
	game.config.bomb_damage = (unsigned long)json_find(root, "bombDamage")->number;

	game.config.units = ft_parse_unit_config(root);

	free_json(root);
}
