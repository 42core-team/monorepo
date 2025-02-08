#include "parse_json.h"

t_obj	*ft_create_unit(t_unit_type type_id)
{
	int unit_count = 0;
	while (game.config.units[unit_count] != NULL)
		unit_count++;
	if ((int)type_id < 1 || (int)type_id > unit_count)
		return NULL;
	if (game.config.units[type_id - 1]->cost > ft_get_my_core()->s_core.balance)
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
	while (game.units[unitsLen])
		unitsLen++;
	game.units = realloc(game.units, sizeof(t_obj *) * (unitsLen + 2));
	game.units[unitsLen] = newUnit;
	game.units[unitsLen + 1] = NULL;

	return newUnit;
}

void	ft_move(t_obj *unit, t_direction direction)
{
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
