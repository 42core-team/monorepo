## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L104

## Description

Immutable info on what each type of unit is able to do.

## Signature

```c
typedef struct s_unit_config
{
	char *name;
	t_unit_type unit_type;
	unsigned long cost;
	unsigned long hp;
	unsigned long dmg_core;
	unsigned long dmg_unit;
	unsigned long dmg_deposit;
	unsigned long dmg_wall;
	t_build_type build_type;
	unsigned long baseActionCooldown;
	unsigned long maxActionCooldown;
	bool can_build;
} t_unit_config;
```

## Parameters

- `name`: The name of the unit.
- `unit_type`: The unit type of the unit.
- `cost`: What the unit costs to create.
- `hp`: How much healthpoints the unit has.
- `dmg_core`: How much damage the unit deals to cores.
- `dmg_unit`: How much damage the unit deals to units.
- `dmg_deposit`: How much damage the unit deals to deposits.
- `dmg_wall`: How much damage the unit deals to walls.
- `dmg_bomb`: How much damage the unit deals to walls.
- `build_type`: The units build type. If "none", the unit can't build.
- `baseActionCooldown`: The time a unit waits between moves if it is not carrying any gems.
- `maxActionCooldown`: The maximum boundary of a units wait time between moves if it's carrying a ton of gems.
- `balancePerCooldownStep`: Defines increase of delay between action executions.

## Examples

## Related
