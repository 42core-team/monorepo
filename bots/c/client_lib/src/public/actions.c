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

void core_action_createUnit(const char *name, char *component, ...)
{
	core_static_ensureCapacity();

	t_action *action = &actions.list[actions.count++];
	memset(action, 0, sizeof(t_action));

	action->type = ACTION_CREATE;

	action->data.create.name = name ? strdup(name) : NULL;

	size_t count = 0;
	size_t capacity = 4;

	action->data.create.components = malloc(sizeof(char *) * capacity);
	if (!action->data.create.components)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	action->data.create.components[0] = NULL;

	va_list args;
	va_start(args, component);

	char *current = component;
	while (current != NULL)
	{
		if (count + 1 >= capacity)
		{
			capacity *= 2;

			char **new_components = realloc(action->data.create.components, sizeof(char *) * capacity);

			if (!new_components)
			{
				va_end(args);
				core_internal_freeStringArray(action->data.create.components);
				action->data.create.components = NULL;
				perror("realloc");
				exit(EXIT_FAILURE);
			}

			action->data.create.components = new_components;
		}

		size_t len = strlen(current) + 1;
		action->data.create.components[count] = malloc(len);
		if (!action->data.create.components[count])
		{
			va_end(args);
			action->data.create.components[count] = NULL;
			core_internal_freeStringArray(action->data.create.components);
			action->data.create.components = NULL;
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		memcpy(action->data.create.components[count], current, len);

		count++;
		action->data.create.components[count] = NULL;

		current = va_arg(args, char *);
	}

	va_end(args);
}

void core_action_move(const t_obj *unit, t_pos pos)
{
	if (!unit) return;
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_MOVE;
	action->data.move.id = unit->id;
	action->data.move.pos = pos;
}

void core_action_attack(const t_obj *attacker, const t_obj *target)
{
	if (!attacker || !target) return;
	core_static_ensureCapacity();
	t_action *action = &actions.list[actions.count++];
	action->type = ACTION_ATTACK;
	action->data.attack.id = attacker->id;
	action->data.attack.target_id = target->id;
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
