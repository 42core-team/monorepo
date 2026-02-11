---
title: "⚙️ function core_print_config_unit(...)"
permalink: "core_print_config_unit"
sidebarTitle: "⚙️ print_config_unit()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/printing.c#L66

## Description

Prints a selected unit config.

[More Info on configs](documentation/configs)

## Signature

```c
void core_print_config_unit(t_unit_type unit_type);
```

## Parameters

- `t_unit_type unit_type`: The type of unit to print the config for

## Return

void

### stdout

This code block may be an old config with values that do not apply to your current event - if you execute the function they will be correct.

```
Unit Warrior (0) Config:
	- Cost: 150
	- HP: 35
	- Damage to Core: 12
	- Damage to Unit: 6
	- Damage to Deposit: 4
	- Damage to Wall: 5
	- Damage to Bombs: 5
	- Build Type: None
	- Base Action Cooldown: 5
	- Max Action Cooldown: 12
	- Balance per Cooldown Step: 12
```

## Examples

```c
printf("Warrior Config: \n");
core_print_config_unit(UNIT_WARRIOR);
printf("Miner Config: \n");
core_print_config_unit(UNIT_MINER);
printf("Carrier Config: \n");
core_print_config_unit(UNIT_CARRIER);
```

## Related

- [🔢 enum e_unit_type](reference/objects/e_unit_type)
- [Configs Documentation](documentation/configs)
