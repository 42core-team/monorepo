#include "core_lib.h"
#include "core_lib_internal.h"

static inline void core_static_ensureCapacity(void)
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
static inline bool core_static_isFriendlyObj(const t_obj *o)
{
	if (o && o->type == OBJ_UNIT) return o->s_unit.team_id == game.my_team_id;
	if (o && o->type == OBJ_CORE) return o->s_core.team_id == game.my_team_id;
	return false;
}

void core_action_createUnit(t_unit_type unit_type)
{
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_CREATE;
	action->data.create.unit_type = unit_type;
}

void core_action_move(const t_obj *unit, t_pos pos)
{
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_MOVE;
	action->data.move.id = unit->id;
	action->data.move.pos = pos;
}
void core_action_moveTowards(const t_obj *unit, t_pos pos)
{
	if (!unit || unit->type != OBJ_UNIT) return;
	if (unit->pos.x == pos.x && unit->pos.y == pos.y) return;
	if (unit->s_unit.action_cooldown != 0) return;

	t_pos posOptionY = (pos.y == unit->pos.y)
							   ? unit->pos
							   : (t_pos){unit->pos.x, (unsigned short)(unit->pos.y + (pos.y > unit->pos.y ? 1 : -1))};
	t_pos posOptionX = (pos.x == unit->pos.x)
							   ? unit->pos
							   : (t_pos){(unsigned short)(unit->pos.x + (pos.x > unit->pos.x ? 1 : -1)), unit->pos.y};

	int posOptionXPriority = 0;
	int posOptionYPriority = 0;

	// 1. + 2. check: out-of-bounds & one axis done
	if (!core_internal_isPosValid(posOptionY) || posOptionY.y == unit->pos.y) posOptionYPriority += 500;
	if (!core_internal_isPosValid(posOptionX) || posOptionX.x == unit->pos.x) posOptionXPriority += 500;

	// 2. check: prioritize larger axis
	if (abs(unit->pos.x - pos.x) > abs(unit->pos.y - pos.y))
		posOptionYPriority++;
	else
		posOptionXPriority++;

	// 3. check: pos emptiness
	t_obj *posOptionXObj = core_get_obj_from_pos(posOptionX);
	t_obj *posOptionYObj = core_get_obj_from_pos(posOptionY);

	if (posOptionXObj) posOptionXPriority += 50;
	if (posOptionYObj) posOptionYPriority += 50;

	// 4. check: obj friendliness check
	if (core_static_isFriendlyObj(posOptionXObj)) posOptionXPriority += 100;
	if (core_static_isFriendlyObj(posOptionYObj)) posOptionYPriority += 100;

	// -----

	if (posOptionXPriority < 250 && posOptionXPriority < posOptionYPriority)
	{
		if (posOptionXObj && !core_static_isFriendlyObj(posOptionXObj))
			core_action_attack_obj(unit, posOptionXObj);
		else if (!posOptionXObj)
			core_action_move(unit, posOptionX);
		return;
	}
	if (posOptionYPriority < 250)
	{
		if (posOptionYObj && !core_static_isFriendlyObj(posOptionYObj))
			core_action_attack_obj(unit, posOptionYObj);
		else if (!posOptionYObj)
			core_action_move(unit, posOptionY);
		return;
	}
}

void core_action_attack(const t_obj *attacker, t_pos target_pos)
{
	if (!attacker) return;
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_ATTACK;
	action->data.attack.id = attacker->id;
	action->data.attack.pos = target_pos;
}
void core_action_attack_obj(const t_obj *attacker, const t_obj *target)
{
	if (!target) return;
	core_action_attack(attacker, target->pos);
}

void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount)
{
	if (!source) return;
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_TRANSFER;
	action->data.transfer.source_id = source->id;
	action->data.transfer.target_pos = target_pos;
	action->data.transfer.amount = amount;
}
void core_action_transferGems_toObj(const t_obj *source, t_obj *target, unsigned long amount)
{
	if (!target) return;
	core_action_transferGems(source, target->pos, amount);
}

void core_action_build(const t_obj *builder, t_pos pos)
{
	if (!builder) return;

	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_BUILD;
	action->data.build.builder_id = builder->id;
	action->data.build.pos = pos;
}
