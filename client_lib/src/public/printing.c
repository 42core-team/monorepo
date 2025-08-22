#include "core_lib.h"

extern t_game game;

void core_print_obj(const t_obj *obj)
{
	if (obj == NULL)
	{
		printf("core_print_object: Received object was NULL!\n");
		return;
	}

	printf("---");
	printf("ID: %lu\n", obj->id);
	printf("Type: %s\n", obj->type == OBJ_CORE ? "Core" : 
		obj->type == OBJ_UNIT ? "Unit" : 
		obj->type == OBJ_RESOURCE ? "Resource" :
		obj->type == OBJ_WALL ? "Wall" : 
		obj->type == OBJ_MONEY ? "Money" : "Unknown");
		// obj->type == OBJ_BOMB ? "Bomb" : "Unknown");
	printf("State: %s\n", obj->state == STATE_UNINITIALIZED ? "Uninitialized" :
		obj->state == STATE_ALIVE ? "Alive" : 
		obj->state == STATE_DEAD ? "Dead" : "Unknown");
	printf("Position: (%hu, %hu)\n", obj->pos.x, obj->pos.y);
	printf("HP: %lu\n", obj->hp);

	switch (obj->type)
	{
		case OBJ_CORE:
			printf("Team ID: %lu\n", obj->s_core.team_id);
			printf("Balance: %lu\n", obj->s_core.balance);
			break;
		case OBJ_UNIT:
			printf("Unit Type: %lu\n", obj->s_unit.unit_type);
			printf("Team ID: %lu\n", obj->s_unit.team_id);
			printf("Balance: %lu\n", obj->s_unit.balance);
			printf("Action Cooldown: %lu\n", obj->s_unit.action_cooldown);
			break;
		case OBJ_RESOURCE:
		case OBJ_MONEY:
			printf("Balance: %lu\n", obj->s_resource_money.balance);
			break;
		// case OBJ_BOMB:
		// 	printf("Countdown: %lu\n", obj->s_bomb.countdown);
		// 	break;
		default:
			break;
	}
}

t_obj **core_print_objs(t_obj **objs)
{
	if (objs == NULL || objs[0] == NULL)
	{
		printf("core_print_objects: Received objects array was NULL or empty!\n");
		return NULL;
	}
	for (int i = 0; objs[i]; i++)
	{
		t_obj *obj = objs[i];
		core_print_object(obj);
		printf("---\n");
	}

	return objs;
}

void core_print_config_unit(t_unit_type unit_type)
{
	t_unit_config *unit_config = core_get_unitConfig(unit_type);
	if (unit_config == NULL)
	{
		printf("Unit type %d not found.\n", unit_type);
		return;
	}

	printf("Unit %s (%d) Config:\n", unit_config->name, unit_type);
	printf("	- Cost: %lu\n", unit_config->cost);
	printf("	- HP: %lu\n", unit_config->hp);
	printf("	- Damage to Core: %lu\n", unit_config->dmg_core);
	printf("	- Damage to Unit: %lu\n", unit_config->dmg_unit);
	printf("	- Damage to Resource: %lu\n", unit_config->dmg_resource);
	printf("	- Damage to Wall: %lu\n", unit_config->dmg_wall);
	char *build_type = "None";
	if (unit_config->build_type == BUILD_TYPE_WALL)
		build_type = "Wall";
	else if (unit_config->build_type == BUILD_TYPE_BOMB)
		build_type = "Bomb";
	printf("	- Build Type: %s\n", build_type);
	printf("	- Base Action Cooldown: %lu\n", unit_config->baseActionCooldown);
	printf("	- Max Action Cooldown: %lu\n", unit_config->maxActionCooldown);
	printf("	- Can Build: %s\n", unit_config->can_build ? "Yes" : "No");
}

void core_print_config_game(void)
{
	printf("Game Config:\n");
	printf("	- Map Grid Size: %lu\n", game.config.gridSize);
	printf("	- Idle Income: %lu\n", game.config.idle_income);
	printf("	- Idle Income Timeout: %lu\n", game.config.idle_income_timeout);
	printf("	- Resource HP: %lu\n", game.config.resource_hp);
	printf("	- Resource Income: %lu\n", game.config.resource_income);
	printf("	- Core HP: %lu\n", game.config.core_hp);
	printf("	- Initial Balance: %lu\n", game.config.initial_balance);
	printf("	- Wall HP: %lu\n", game.config.wall_hp);
	printf("	- Wall Build Cost: %lu\n", game.config.wall_build_cost);
	printf("	- Bomb HP: %lu\n", game.config.bomb_hp);
	printf("	- Bomb Countdown: %lu\n", game.config.bomb_countdown);
	printf("	- Bomb Throw Cost: %lu\n", game.config.bomb_throw_cost);
	printf("	- Bomb Reach: %lu\n", game.config.bomb_reach);
	printf("	- Bomb Damage: %lu\n", game.config.bomb_damage);
}

void core_print_config(void)
{
	core_print_config_game();

	for (int i = 0; game.config.units[i] != NULL; i++)
	{
		core_print_config_unit(game.config.units[i]->unit_type);
	}
}
