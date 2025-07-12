#include "core_lib.h"
#include "core_lib_internal.h"

void core_internal_reset_actions(void)
{
	free(actions.list);
	actions.list = NULL;
	actions.count = 0;
	actions.capacity = 0;
}

static bool core_static_isMyCore(t_obj *obj)
{
	return obj && obj->type == OBJ_CORE && obj->s_core.team_id == game.my_team_id;
}
static t_obj *core_static_get_myCore(void)
{
	return core_get_obj_customCondition_first(core_static_isMyCore);
}

static void core_static_ensureCapacity(void)
{
	if (actions.list == NULL)
	{
		actions.capacity = 8;
		actions.list = malloc(sizeof(t_action) * actions.capacity);
	}
	else if (actions.count >= actions.capacity)
	{
		actions.capacity *= 2;
		actions.list = realloc(actions.list, sizeof(t_action) * actions.capacity);
	}
}

t_obj *core_action_createUnit(t_unit_type unit_type)
{
	int unit_count = 0;
	while (game.config.units != NULL && game.config.units[unit_count] != NULL)
		unit_count++;
	if ((int)unit_type < 0 || (int)unit_type >= unit_count)
		return NULL;
	if (!core_static_get_myCore() || !game.config.units || !game.config.units[unit_type] ||
		game.config.units[unit_type]->cost > core_static_get_myCore()->s_core.balance)
		return NULL;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_CREATE;
	action->data.create.unit_type = unit_type;

	t_obj *newUnit = malloc(sizeof(t_obj));
	newUnit->s_unit.unit_type = unit_type;
	newUnit->s_unit.team_id = game.my_team_id;
	newUnit->type = OBJ_UNIT;
	newUnit->id = 0;
	newUnit->state = STATE_UNINITIALIZED;
	newUnit->data = NULL;

	int objLen = 0;
	while (game.objects && game.objects[objLen])
		objLen++;
	game.objects = realloc(game.objects, sizeof(t_obj *) * (objLen + 2));
	game.objects[objLen] = newUnit;
	game.objects[objLen + 1] = NULL;

	core_static_get_myCore()->s_core.balance -= game.config.units[unit_type]->cost;

	return newUnit;
}

void core_action_move(t_obj *unit, t_pos pos)
{
	if (unit->s_unit.move_cooldown != 0)
		return;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_MOVE;
	action->data.move.id = unit->id;
	action->data.move.pos = pos;
}

void core_action_attack(t_obj *attacker, t_pos target_pos)
{
	if (!attacker)
		return;
	if (attacker->type != OBJ_UNIT)
		return;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_ATTACK;
	action->data.attack.id = attacker->id;
	action->data.attack.pos = target_pos;
}

void core_action_transferMoney(t_obj *source, t_pos target_pos, unsigned long amount)
{
	if (!source)
		return;
	if (source->type != OBJ_CORE && source->type != OBJ_UNIT)
		return;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_TRANSFER;
	action->data.transfer.source_id = source->id;
	action->data.transfer.target_pos = target_pos;
	action->data.transfer.amount = amount;
}

void core_action_build(t_obj *builder, t_pos pos)
{
	if (!builder || builder->type != OBJ_UNIT || (builder->s_unit.unit_type != UNIT_BUILDER && builder->s_unit.unit_type != UNIT_BOMBERMAN))
		return;
	if (builder->s_unit.unit_type == UNIT_BUILDER && builder->s_unit.balance < game.config.wall_build_cost)
		return;
	if (builder->s_unit.unit_type == UNIT_BOMBERMAN && builder->s_unit.balance < game.config.bomb_throw_cost)
		return;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_BUILD;
	action->data.build.builder_id = builder->id;
	action->data.build.pos = pos;

	if (builder->s_unit.unit_type == UNIT_BUILDER)
		builder->s_unit.balance -= game.config.wall_build_cost;
	if (builder->s_unit.unit_type == UNIT_BOMBERMAN)
		builder->s_unit.balance -= game.config.bomb_throw_cost;
}
