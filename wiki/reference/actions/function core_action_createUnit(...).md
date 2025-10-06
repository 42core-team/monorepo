## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L24

## Description

Create a new unit of specified type.

Units are spawned as close to your core as possible - but if all positions directly next to your core are occupied, a flood fill algorithm will be used looking for the next empty space and the unit will be placed there.

## Signature

```c
t_obj *core_action_createUnit(t_unit_type unit_type);
```

## Parameters

- `unit_type`: The type of unit to create
- `return`: A newly created, uninitialized unit object or NULL if the unit could not be created.

## Examples

```c
if (core_get_objs_filter_count(ft_is_own_team_warrior) < 3)
{
	core_action_createUnit(UNIT_WARRIOR);
}
```

## Related
