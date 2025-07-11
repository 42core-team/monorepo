#include "core_lib.h"

extern t_game game;

void core_util_print_object(t_obj *obj)
{
	printf("---");
	printf("ID: %lu\n", obj->id);
	printf("Type: %s\n", obj->type == OBJ_CORE ? "Core" : 
		obj->type == OBJ_UNIT ? "Unit" : 
		obj->type == OBJ_RESOURCE ? "Resource" :
		obj->type == OBJ_WALL ? "Wall" : 
		obj->type == OBJ_MONEY ? "Money" :
		obj->type == OBJ_BOMB ? "Bomb" : "Unknown");
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
			printf("Next Movement Opp: %lu\n", obj->s_unit.next_movement_opp);
			break;
		case OBJ_RESOURCE:
		case OBJ_MONEY:
			printf("Balance: %lu\n", obj->s_resource_money.balance);
			break;
		case OBJ_BOMB:
			printf("Countdown: %lu\n", obj->s_bomb.countdown);
			break;
		default:
			break;
	}
}

void core_util_print_objects(bool (*condition)(t_obj *))
{
	if (game.objects == NULL)
		return;
	for (int i = 0; game.objects[i]; i++)
	{
		t_obj *obj = game.objects[i];
		if (condition && !condition(obj))
			continue;
		core_util_print_object(obj);
		printf("---\n");
	}
}
