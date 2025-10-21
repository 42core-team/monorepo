=====
HEADING="🧩 struct s_obj"
PERMALINK="s_obj"
SIDEBAR_HEADING="🧩 s_obj"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L36

## Description

Game object structure representing all the things that can be present on the game field.

- There are never two objects with the same id.
- There are never more than one object at a given [grid position](reference/objects/s_pos).

> [!WARNING]
> The memory location of objects may change between ticks. To store references to objects between ticks, it's recommended to use their id.

There is no limit to how many gems units, cores or gem piles can hold.

> [!TIP]
> The `void *data` field is **especially powerful & useful**. The library will never touch this field, it's yours to mess with for whatever you want.\
It can already safely be set when a unit is still uninitialized, so immediately after unit creation, in the same tick. It's generally used to store specific jobs, tasks or targets of the unit in the easiest way possible.

## Signature

```c
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
			t_unit_type unit_type;
			unsigned long team_id;
			unsigned long gems;
			unsigned long action_cooldown;
		} s_unit;
		struct
		{
			unsigned long gems;
		} s_deposit_gems_pile;
		struct
		{
			unsigned long countdown;
		} s_bomb;
	};
} t_obj;
```

## Parameters

- `t_obj_type type`: Type of the obj
- `void *data`: Custom data, *save whatever you want here*, it will persist across ticks.
- `unsigned long id`: The unique id of the obj
- `t_pos pos`: The position of the obj
- `unsigned long hp`: The current healthpoints of the obj



- `unsigned long s_core.team_id`: The id of the team that owns the core.
- `unsigned long s_core.gems`: The current gems stored in the core.
- `unsigned long s_core.spawn_cooldown`: Countdown to the next tick the core can spawn a unit, defined by core_spawn_cooldown in the config.



- `t_unit_type s_unit.unit_type`: Which type of unit this is.
- `unsigned long s_unit.team_id`: The id of the team that owns the unit.
- `unsigned long s_unit.gems`: The amount of gems the unit is carrying.
- `unsigned long s_unit.action_cooldown`: Countdown to the next tick the unit can move, defined by it's action cooldown & how many gems it's carrying.



- `unsigned long s_deposit_gems_pile.gems`: The amount of gems the deposit or gem pile contains.



- `unsigned long s_bomb.countdown`: The time until the bomb will explode. **TAKE COVER!**

Walls don't have any special properties so they don't need a unionized struct, deposits and gem piles share the same additional properties so they share the same unionized struct (`s_deposit_gems_pile`).

## Examples

```c
// move units
t_obj **units = ft_get_units_own();
for (int i = 0; units && units[i]; i++)
{
	t_obj *obj = units[i];

	switch ((int)obj->s_unit.unit_type)
	{
	case UNIT_WARRIOR:
		// ...
		break;

	case UNIT_MINER:
		// ...
		break;
	}
}
free(units);
```

## Related

- [🔢 enum e_obj_type](reference/objects/e_obj_type)
- [🧩 struct s_pos](reference/objects/s_pos)
- [🔢 enum e_unit_type](reference/objects/e_unit_type)
