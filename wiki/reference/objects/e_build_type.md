=====
HEADING="🔢 enum e_build_type"
PERMALINK="e_build_type"
SIDEBAR_HEADING="🔢 e_build_type"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L86

## Description

Determines whether and what the unit will build if you use it as the builder argument when executing a build action.

This enum is only used in the config to indicate each units building capabilities.

## Signature

```c
typedef enum e_build_type
{
	BUILD_TYPE_NONE = 0,
	BUILD_TYPE_WALL = 1,
	BUILD_TYPE_BOMB = 2
} t_build_type;
```

## Parameters

- **None**: The default unit build setting, if you call `core_action_build` on this, it simply won't do anything.
- **Wall**: This unit is able to build walls.
- **Bomb**: This unit is able to create bombs.

## Examples

```c
// building logic
if (core_get_unitConfig(obj->s_unit.unit_type)->build_type == BUILD_TYPE_NONE)
	continue;
```

## Related
