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

void	ft_travel_to(t_unit *unit, unsigned long x, unsigned long y)
{
	ft_travel_to_id(unit->id, x, y);
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

void	ft_travel_dir(t_unit *unit, long x, long y)
{
	ft_travel_dir_id(unit->id, x, y);
}

void	ft_create_type_id(unsigned long type_id)
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

void	ft_create(t_unit_config *unit_config)
{
	ft_create_type_id(unit_config->type_id);
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

void	ft_attack(t_unit *attacker, t_unit *target)
{
	ft_attack_id(attacker->id, target->id);
}
