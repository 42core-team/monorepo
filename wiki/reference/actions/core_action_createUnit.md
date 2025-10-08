=====
HEADING="⚙️ function core_action_createUnit(...)"
PERMALINK="core_action_createUnit"
SIDEBAR_HEADING="⚙️ core_action_createUnit()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L24

## Description

Creates a new unit of specified type.

Units are spawned as close to your core as possible - but if all positions directly next to your core are occupied, a flood fill algorithm will be used looking for the next empty space and the unit will be placed there.

## Signature

```c
void core_action_createUnit(t_unit_type unit_type);
```

## Parameters

- `t_unit_type unit_type`: The type of unit to create

## Return

void

## Examples

```c
if (core_get_objs_filter_count(ft_is_own_team_warrior) < 3)
{
	core_action_createUnit(UNIT_WARRIOR);
}
```

## Related
