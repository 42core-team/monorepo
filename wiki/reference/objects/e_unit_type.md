=====
HEADING="🔢 enum e_unit_type"
PERMALINK="e_unit_type"
SIDEBAR_HEADING="🔢 e_unit_type"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/units.h#L5

## Description

Type of unit. Which are available differs from event to event.

There is an overview of all the unit types and their properties and settings in the config.

> **TIP**: Type `UNIT_` and autocomplete will show you all the available units.

## Signature

```c
typedef enum e_unit_type
{
	UNIT_WARRIOR = 0,
	UNIT_MINER = 1,
	UNIT_CARRIER = 2,
	// ... (it depends)
} t_unit_type;
```

## Parameters

To find out what each unit is good or bad at, check out the config.

Basic units that are in almost every config / event are:

- **Warrior**: Offensive melee unit, good at attacking other units and cores.
- **Miner**: Pickaxe-wielding worker, good at destroying gem deposits & walls.
- **Carrier**: Light-footed transporter, fast & does not get slowed down when carrying lots of money as quickly as other units.

Note this info may be inaccurate, please check your config.

## Examples

```c
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
```

## Related

- [🧩 struct s_obj](../objects/s_obj)
- [🧩 struct s_unit_config](../config/s_unit_config)
