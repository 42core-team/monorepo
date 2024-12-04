#include "parse_json.h"

void	ft_travel_to_id(unsigned long id, unsigned long x, unsigned long y)
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

	(*actions)[*count].id = id;
	(*actions)[*count].is_vector = false;
	(*actions)[*count].x = x;
	(*actions)[*count].y = y;
	(*count)++;
}

void	ft_travel_to(t_obj *unit, unsigned long x, unsigned long y)
{
	if (unit == NULL)
		return;
	if (unit->type != OBJ_UNIT)
		return (ft_print_error("OBJ is not type of UNIT", __func__));
	ft_travel_to_id(unit->id, x, y);
}

void	ft_travel_to_id_obj(unsigned long id, t_obj *target)
{
	ft_travel_to_id(id, target->x, target->y);
}

void	ft_travel_to_obj(t_obj *unit, t_obj *target)
{
	if (unit == NULL || target == NULL)
		return;
	if (unit->type != OBJ_UNIT)
		return (ft_print_error("OBJ is not type of UNIT", __func__));
	ft_travel_to(unit, target->x, target->y);
}

void	ft_travel_dir_id(unsigned long id, long x, long y)
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

	(*actions)[*count].id = id;
	(*actions)[*count].is_vector = true;
	(*actions)[*count].x = x;
	(*actions)[*count].y = y;
	(*count)++;
}

void	ft_travel_dir(t_obj *unit, long x, long y)
{
	if (unit == NULL)
		return;
	if (unit->type != OBJ_UNIT)
		return (ft_print_error("OBJ is not type of UNIT", __func__));
	ft_travel_dir_id(unit->id, x, y);
}

t_obj	*ft_create_unit(t_unit_type type_id)
{
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

	int unitsLen = 0;
	while (game.units[unitsLen])
		unitsLen++;
	game.units = realloc(game.units, sizeof(t_obj *) * (unitsLen + 2));
	game.units[unitsLen] = newUnit;
	game.units[unitsLen + 1] = NULL;

	return newUnit;
}

void	ft_attack_id(unsigned long attacker_id, unsigned long target_id)
{
	t_action_attack	**actions = &game.actions.attacks;
	unsigned int	*count = &game.actions.attacks_count;

	if (!*actions)
	{
		*actions = malloc(sizeof(t_action_attack) * 2);
		*count = 0;
	}
	else
		*actions = realloc(*actions, sizeof(t_action_attack) * (*count + 2));
	(*actions)[*count + 1].attacker_id = 0;

	(*actions)[*count].attacker_id = attacker_id;
	(*actions)[*count].target_id = target_id;
	(*count)++;
}

void	ft_attack(t_obj *attacker_unit, t_obj *target_obj)
{
	if (attacker_unit == NULL || target_obj == NULL)
		return;
	if (attacker_unit->type != OBJ_UNIT)
		return ft_print_error("Attacker OBJ is not type of UNIT", __func__);
	switch (target_obj->type)
	{
	case OBJ_RESOURCE:
		break;
	case OBJ_CORE:
		if (attacker_unit->s_unit.team_id == target_obj->s_core.team_id)
		{
			printf("Problem with unit %lu and core\n", attacker_unit->id);
			return ft_print_error("You are trying to attack yourself", __func__);
		}
		break;
	case OBJ_UNIT:
		if (attacker_unit->s_unit.team_id == target_obj->s_unit.team_id)
		{
			printf("Problem with unit %lu\n", attacker_unit->id);
			return ft_print_error("You are trying to attack yourself", __func__);
		}
		break;
	}
	ft_attack_id(attacker_unit->id, target_obj->id);
}

void	ft_travel_attack(t_obj *attacker_unit, t_obj *attack_obj)
{
	if (!attacker_unit || !attack_obj || attacker_unit->state != STATE_ALIVE || attack_obj->state != STATE_ALIVE)
		return ;

	double dist = ft_distance(attacker_unit, attack_obj);
	t_unit_config *attacker_config = ft_get_unit_config(attacker_unit->s_unit.type_id);
	if (!attacker_config)
		ft_attack(attacker_unit, attack_obj);

	if (dist > attacker_config->max_range)
		ft_travel_to_obj(attacker_unit, attack_obj);
	else if (dist < attacker_config->min_range)
		ft_travel_to_obj(attacker_unit, ft_get_my_core());
	else
		ft_travel_to_obj(attacker_unit, attacker_unit);

	ft_attack(attacker_unit, attack_obj);
}
