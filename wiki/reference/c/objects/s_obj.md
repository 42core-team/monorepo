---
title: "🧩 struct s_obj"
permalink: "s_obj"
sidebarTitle: "🧩 s_obj"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Represents every object currently visible in the game state. IDs are unique, and at most one object occupies a grid position.

> [!WARNING]
> Object memory may move between ticks. Store an object's `id` and retrieve it again with `core_get_obj_from_id` instead of keeping a `t_obj *` across callbacks.

The library owns each object and the strings and component arrays inside it. Do not free them. The `void *data` field is the exception: it is reserved for your strategy and persists with the object across state updates, so you own any memory you put there.

## Signature

```c
typedef struct s_unit_properties
{
	unsigned long hp;
	int base_action_cooldown;
	unsigned long gems_per_cooldown_step;
	unsigned long max_gems;
	int damage_reduction_percent;
	unsigned long damage_core;
	unsigned long damage_unit;
	unsigned long damage_object;
	unsigned long post_spawn_core_cooldown;
} t_unit_properties;

typedef struct s_obj
{
	t_obj_type type;
	void *data;
	unsigned long id;
	t_pos pos;
	unsigned long hp;
	union
	{
		struct
		{
			unsigned long team_id;
			unsigned long gems;
			unsigned long spawn_cooldown;
		} s_core;
		struct
		{
			unsigned long team_id;
			unsigned long gems;
			long action_cooldown;
			char **components;
			t_unit_properties properties;
			char *name;
		} s_unit;
		struct
		{
			unsigned long gems;
		} s_deposit_gems_pile;
	};
} t_obj;
```

## Common fields

- `type`: Which union member is valid.
- `data`: Strategy-owned storage untouched by the library.
- `id`: Unique object ID.
- `pos`: Current grid position.
- `hp`: Current health, not necessarily the object's starting health.

## Core fields

- `s_core.team_id`: Team that owns the core.
- `s_core.gems`: Gems currently stored by the core.
- `s_core.spawn_cooldown`: Ticks until this core may create another unit.

## Unit fields

- `s_unit.team_id`: Team that owns the unit.
- `s_unit.gems`: Gems currently carried by the unit.
- `s_unit.action_cooldown`: Ticks until the unit can act. Zero and negative values mean ready.
- `s_unit.components`: `NULL`-terminated array of the component IDs selected when the unit was created.
- `s_unit.properties`: Final properties calculated from the event defaults and components. See the [Unit Builder property guide](documentation/unit_builder).
- `s_unit.name`: Custom or generated name.

## Deposit and gem-pile fields

- `s_deposit_gems_pile.gems`: Gems contained by a deposit or gem pile.

Walls have no additional union fields.

## Example

```c
for (size_t i = 0; game.objects && game.objects[i]; i++)
{
	t_obj *obj = game.objects[i];
	if (obj->type == OBJ_UNIT && obj->s_unit.team_id == game.my_team_id)
		printf("%s: %lu HP, %lu gems\n", obj->s_unit.name, obj->hp, obj->s_unit.gems);
}
```

## Related

- [Unit Builder](documentation/unit_builder)
- [🔢 enum e_obj_type](reference/c/objects/e_obj_type)
- [🧩 struct s_pos](reference/c/objects/s_pos)
