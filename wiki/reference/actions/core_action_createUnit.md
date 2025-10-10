=====
HEADING="⚙️ function core_action_createUnit(...)"
PERMALINK="core_action_createUnit"
SIDEBAR_HEADING="⚙️ action_createUnit()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/actions.c#L24

## Description

Creates a new unit of specified type.

Your core needs to own a certain amount of money to be able to create a unit. This amount is different depending on the [type of unit](reference/objects/e_unit_type) and can be found in the [config](documentation/configs).

Cores with a spawn cooldown that isn't 0 are unable to spawn new units. [More info on spawn cooldowns](documentation/cooldowns).

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

- [🔢 enum e_unit_type](reference/objects/e_unit_type)
