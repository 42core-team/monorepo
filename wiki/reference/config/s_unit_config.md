=====
HEADING="🧩 struct s_unit_config"
PERMALINK="s_unit_config"
SIDEBAR_HEADING="🧩 s_unit_config"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L93

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
	unsigned long dmg_bomb;
	t_build_type build_type;
	unsigned long baseActionCooldown;
	unsigned long maxActionCooldown;
	unsigned long balancePerCooldownStep;
} t_unit_config;
```

## Parameters

- `char *name`: The name of the unit.
- `t_unit_type unit_type`: The unit type of the unit.
- `unsigned long cost`: What the unit costs to create.
- `unsigned long hp`: How much healthpoints the unit has.
- `unsigned long dmg_core`: How much damage the unit deals to cores.
- `unsigned long dmg_unit`: How much damage the unit deals to units.
- `unsigned long dmg_deposit`: How much damage the unit deals to deposits.
- `unsigned long dmg_wall`: How much damage the unit deals to walls.
- `unsigned long dmg_bomb`: How much damage the unit deals to bombs.
- `t_build_type build_type`: The units build type. If "none", the unit can't build.
- `unsigned long baseActionCooldown`: The time a unit waits between moves if it is not carrying any gems.
- `unsigned long maxActionCooldown`: The maximum boundary of a units wait time between moves if it's carrying a ton of gems.
- `unsigned long balancePerCooldownStep`: Defines increase of delay between action executions.

## Examples

```c
t_unit_config *uconf = core_get_unitConfig(UNIT_MINER);
if (ft_get_core_own()->s_core.gems >= uconf->cost)
{
	core_action_createUnit(UNIT_MINER);
}
```

## Related
