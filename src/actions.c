#include "parse_json.h"

t_obj	*ft_create_unit(t_unit_type type_id)
{
	int unit_count = 0;
	while (game.config.units != NULL && game.config.units[unit_count] != NULL)
		unit_count++;
	if ((int)type_id < 0 || (int)type_id >= unit_count)
		return NULL;
	if (!ft_get_my_core() || !game.config.units || !game.config.units[type_id] || \
			game.config.units[type_id]->cost > ft_get_my_core()->s_core.balance)
		return NULL;

	t_action_create	**actions = &game.actions.creates;
	unsigned int	*count = &game.actions.creates_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_create) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_create) * (*count + 2));
	(*actions)[*count + 1].type_id = 0;

	(*actions)[*count].type_id = type_id;
	(*count)++;

	t_obj *newUnit = malloc(sizeof(t_obj));
	newUnit->s_unit.type_id = type_id;
	newUnit->s_unit.team_id = game.my_team_id;
	newUnit->type = OBJ_UNIT;
	newUnit->id = 0;
	newUnit->state = STATE_UNINITIALIZED;
	newUnit->data = NULL;

	int unitsLen = 0;
	while (game.units && game.units[unitsLen])
		unitsLen++;
	game.units = realloc(game.units, sizeof(t_obj *) * (unitsLen + 2));
	game.units[unitsLen] = newUnit;
	game.units[unitsLen + 1] = NULL;

	ft_get_my_core()->s_core.balance -= game.config.units[type_id]->cost;

	return newUnit;
}

void	ft_move(t_obj *unit, t_direction direction)
{
	if (unit->s_unit.next_movement_opp != 0)
		return;

	t_action_travel	**actions = &game.actions.travels;
	unsigned int	*count = &game.actions.travels_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_travel) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_travel) * (*count + 2));
	(*actions)[*count + 1].id = 0;

	(*actions)[*count].id = unit->id;
	(*actions)[*count].direction = direction;
	(*count)++;
}
void	ft_travel_to_pos(t_obj *unit, t_pos pos)
{
	bool biggestAxisX = abs(unit->pos.x - pos.x) > abs(unit->pos.y - pos.y);
	if (biggestAxisX)
	{
		if (unit->pos.x < pos.x)
			ft_move(unit, DIR_RIGHT);
		else if (unit->pos.x > pos.x)
			ft_move(unit, DIR_LEFT);
	}
	else
	{
		if (unit->pos.y < pos.y)
			ft_move(unit, DIR_DOWN);
		else if (unit->pos.y > pos.y)
			ft_move(unit, DIR_UP);
	}
}

void	ft_attack(t_obj *attacker, t_direction direction)
{
	if (!attacker)
		return;
	if (attacker->type != OBJ_UNIT)
		return;

	t_action_attack	**actions = &game.actions.attacks;
	unsigned int	*count = &game.actions.attacks_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_attack) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_attack) * (*count + 2));
	(*actions)[*count + 1].id = 0;

	(*actions)[*count].id = attacker->id;
	(*actions)[*count].direction = direction;
	(*count)++;
}

void	ft_drop_money(t_obj *source, t_pos target_pos, unsigned long amount)
{
	if (!source || source->s_unit.balance < amount)
		return;
	if (source->type != OBJ_CORE && source->type != OBJ_UNIT)
		return;

	t_action_transfer_money	**actions = &game.actions.transfer_moneys;
	unsigned int			*count = &game.actions.transfer_moneys_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_transfer_money) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_transfer_money) * (*count + 2));
	(*actions)[*count + 1].source_id = 0;

	(*actions)[*count].source_id = source->id;
	(*actions)[*count].target_pos = target_pos;
	(*actions)[*count].amount = amount;
	(*count)++;
}
void	ft_transfer_money(t_obj *source, t_obj *target, unsigned long amount)
{
	if (!source || !target)
		return;
	if (source->type != OBJ_CORE && source->type != OBJ_UNIT)
		return;
	if (target->type != OBJ_CORE && target->type != OBJ_UNIT)
		return;

	t_pos target_pos = target->pos;
	ft_drop_money(source, target_pos, amount);
}

void	ft_build(t_obj *builder, t_pos pos)
{
	if (!builder || builder->type != OBJ_UNIT || builder->s_unit.type_id != UNIT_BUILDER)
		return;
	if (builder->s_unit.balance < game.config.wall_build_cost)
		return;

	t_action_build	**actions = &game.actions.builds;
	unsigned int	*count = &game.actions.builds_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_build) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_build) * (*count + 2));
	(*actions)[*count + 1].id = 0;

	(*actions)[*count].id = builder->id;
	(*actions)[*count].pos = pos;
	(*count)++;
}
