#include "bot.h"

static int ft_sign(int num)
{
	if (num > 0) return 1;
	if (num < 0) return -1;
	return 0;
}

static bool ft_is_own_obj(t_obj *obj)
{
	if (!obj) return false;
	if (obj->type == OBJ_UNIT) return obj->s_unit.team_id == game.my_team_id;
	if (obj->type == OBJ_CORE) return obj->s_core.team_id == game.my_team_id;
	return false;
}

static bool ft_is_pos_blocked(t_pos pos)
{
	t_obj *obj = core_get_obj_from_pos(pos);
	if (obj) return true;
	if (pos.x >= game.config.gridSize || pos.y >= game.config.gridSize) return true;
	return false;
}

void ft_pathfind(t_obj *moving_unit, t_pos target_pos)
{
	int dx = target_pos.x - moving_unit->pos.x;
	int dy = target_pos.y - moving_unit->pos.y;

	t_pos nextPosOptionHorizontal = {moving_unit->pos.x + ft_sign(dx), moving_unit->pos.y};
	t_pos nextPosOptionVertical = {moving_unit->pos.x, moving_unit->pos.y + ft_sign(dy)};

	t_obj *nextPosOptionHorizontalObj = core_get_obj_from_pos(nextPosOptionHorizontal);
	t_obj *nextPosOptionVerticalObj = core_get_obj_from_pos(nextPosOptionVertical);

	bool nextPosOptionHorizontalBlocked = ft_is_pos_blocked(nextPosOptionHorizontal);
	bool nextPosOptionVerticalBlocked = ft_is_pos_blocked(nextPosOptionVertical);

	if (!nextPosOptionHorizontalBlocked)
		core_action_move(moving_unit, nextPosOptionHorizontal);
	else if (!nextPosOptionVerticalBlocked)
		core_action_move(moving_unit, nextPosOptionVertical);
	else if (!ft_is_own_obj(nextPosOptionHorizontalObj))
		core_action_attack(moving_unit, nextPosOptionHorizontal);
	else if (!ft_is_own_obj(nextPosOptionVerticalObj))
		core_action_attack(moving_unit, nextPosOptionVertical);
}
