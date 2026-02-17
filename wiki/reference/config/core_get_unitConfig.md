---
title: "⚙️ function core_get_unitConfig(...)"
permalink: "core_get_unitConfig"
sidebarTitle: "⚙️ get_unitConfig()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/get.c#L23

## Description

Get the unit config for a specific unit type.

[More Info on configs](documentation/configs)

## Signature

```c
t_unit_config *core_get_unitConfig(t_unit_type type);
```

## Parameters

- `t_unit_type type`: The type of unit to get the config for

## Return

- `t_unit_config *`: The unit config or NULL if no such unit type or unit config exists.

## Examples

```c
t_unit_config *uconf = core_get_unitConfig(UNIT_MINER);
if (ft_get_core_own()->s_core.gems >= uconf->cost)
{
	core_action_createUnit(UNIT_MINER);
}
```

## Related

- [🧩 struct s_unit_config](reference/config/s_unit_config)
- [🔢 enum e_unit_type](reference/objects/e_unit_type)
- [Configs Documentation](documentation/configs)
