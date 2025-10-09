=====
HEADING="⚙️ function core_print_config(...)"
PERMALINK="core_print_config"
SIDEBAR_HEADING="⚙️ print_config()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L114

## Description

Prints the entire game config and all unit configs.

[More Info on configs](../../documentation/configs)

## Signature

```c
void core_print_config(void);
```

## Parameters

void

## Return

void

### stdout

This code block may be an old config with values that do not apply to your current event - if you execute the function they will be correct.

```
Game Config:
	- Map Grid Size: 20
	- Idle Income: 2
	- Idle Income Timeout: 1000
	- Deposit HP: 50
	- Deposit Income: 150
	- Core HP: 200
	- Initial gems: 200
	- Wall HP: 50
	- Wall Build Cost: 20
	- Bomb Countdown: 10
	- Bomb Throw Cost: 50
	- Bomb Reach: 3
	- Bomb Damage to Cores: 50
	- Bomb Damage to Units: 30
	- Bomb Damage to Deposits: 40
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
Unit Miner (1) Config:
	...
Unit Carrier (2) Config:
	...
```

## Examples

```c
void ft_on_tick(unsigned long tick)
{
	if (tick == 0)
	{
		core_print_config();
	}
// ...
```

## Related

- [Configs Documentation](../../documentation/configs)
