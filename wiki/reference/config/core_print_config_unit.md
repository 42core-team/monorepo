=====
HEADING="⚙️ function core_print_config_unit(...)"
PERMALINK="core_print_config_unit"
SIDEBAR_HEADING="⚙️ print_config_unit()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L70

## Description

Prints a selected unit config.

## Signature

```c
void core_print_config_unit(t_unit_type unit_type);
```

## Parameters

- `t_unit_type unit_type`: The type of unit to print the config for

## Return

void

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
