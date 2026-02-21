#include "core_lib.h"
#include "core_lib_internal.h"

static inline bool core_static_isFriendlyObj(const t_obj *o)
{
	if (o && o->type == OBJ_UNIT) return o->s_unit.team_id == game.my_team_id;
	if (o && o->type == OBJ_CORE) return o->s_core.team_id == game.my_team_id;
	return false;
}

void core_action_createUnit(t_unit_type unit_type)
{
	grpc_bridge_create_unit(unit_type);
}

void core_action_move(const t_obj *unit, t_pos pos)
{
	if (!unit) return;
	grpc_bridge_move(unit->id, pos.x, pos.y);
}

void core_action_pathfind(const t_obj *unit, t_pos pos)
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
			core_action_attack(unit, posOptionXObj);
		else if (!posOptionXObj)
			core_action_move(unit, posOptionX);
		return;
	}
	if (posOptionYPriority < 250)
	{
		if (posOptionYObj && !core_static_isFriendlyObj(posOptionYObj))
			core_action_attack(unit, posOptionYObj);
		else if (!posOptionYObj)
			core_action_move(unit, posOptionY);
		return;
	}
}

void core_action_attack(const t_obj *attacker, const t_obj *target)
{
	if (!attacker || !target) return;
	grpc_bridge_attack(attacker->id, target->id);
}

void core_action_transferGems(const t_obj *source, t_pos target_pos, unsigned long amount)
{
	if (!source) return;
	grpc_bridge_transfer_gems(source->id, target_pos.x, target_pos.y, amount);
}

void core_action_build(const t_obj *builder, t_pos pos)
{
	if (!builder) return;
	grpc_bridge_build(builder->id, pos.x, pos.y);
}
