#include "parse_json.h"

/**
 * @brief Lets a unit travel to a specific coordinate. Same as ft_travel_to, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param id Which unit should travel.
 * @param x To which x coordinate the unit should travel.
 * @param y To which y coordinate the unit should travel.
 */
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

/**
 * @brief Lets a unit travel to a specific coordinate. Same as ft_travel_to_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param unit Pointer to the unit that should travel.
 * @param x To which x coordinate the unit should travel.
 * @param y To which y coordinate the unit should travel.
 */
void	ft_travel_to(t_obj *unit, unsigned long x, unsigned long y)
{
	if (unit->type != OBJ_UNIT)
	{
		ft_print_error("OBJ is not type of UNIT", __func__);
		return;
	}
	ft_travel_to_id(unit->id, x, y);
}

/**
 * @brief Lets a unit start to travel into a specific direction. Same as ft_travel_dir, besides that this function takes an id instead of a pointer to a unit. When x and y are both 0, the unit will stop traveling.
 *
 * @param id Which unit should travel.
 * @param x x vector of the direction the unit should travel.
 * @param y y vector of the direction the unit should travel.
 */
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

/**
 * @brief Lets a unit start to travel into a specific direction. Same as ft_travel_dir_id, besides that this function takes a pointer to a unit instead of an id. When x and y are both 0, the unit will stop traveling.
 *
 * @param id Which unit should travel.
 * @param x x vector of the direction the unit should travel.
 * @param y y vector of the direction the unit should travel.
 */
void	ft_travel_dir(t_obj *unit, long x, long y)
{
	if (unit->type != OBJ_UNIT)
	{
		ft_print_error("OBJ is not type of UNIT", __func__);
		return;
	}
	ft_travel_dir_id(unit->id, x, y);
}

/**
 * @brief Creates a unit of a specific type. Same as ft_create, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param type_id Which type of unit should be created.
 */
void	ft_create_type_id(t_unit_type type_id)
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
}

/**
 * @brief Creates a unit of a specific type. Same as ft_create_type_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param unit_config Pointer to the unit config that should be created.
 */
void	ft_create(t_unit_config *unit_config)
{
	ft_create_type_id(unit_config->type_id);
}

/**
 * @brief Lets a unit attack another unit. Same as ft_attack, besides that this function takes an id instead of a pointer to a unit.
 *
 * @param attacker_id Which unit should be used to attack.
 * @param target_id Which unit should be attacked.
 */
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

/**
 * @brief Lets a unit attack another unit. Same as ft_attack_id, besides that this function takes a pointer to a unit instead of an id.
 *
 * @param attacker_unit Pointer to the unit that should be used to attack.
 * @param target_obj Pointer to the obj that should be attacked.
 */
void	ft_attack(t_obj *attacker_unit, t_obj *target_obj)
{
	if (attacker_unit->type != OBJ_UNIT)
		return ft_print_error("Attacker OBJ is not type of UNIT", __func__);
	switch (target_obj->type)
	{
	case OBJ_RESOURCE:
		break;
	case OBJ_CORE:
		if (attacker_unit->s_unit.team_id == target_obj->s_core.team_id)
			return ft_print_error("You are trying to attack yourself", __func__);
		break;
	case OBJ_UNIT:
		if (attacker_unit->s_unit.team_id == target_obj->s_unit.team_id)
			return ft_print_error("You are trying to attack yourself", __func__);
		break;
	}
	ft_attack_id(attacker_unit->id, target_obj->id);
}
