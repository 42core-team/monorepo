=====
HEADING="⚙️ function core_print_config_game(...)"
PERMALINK="core_print_config_game"
SIDEBAR_HEADING="⚙️ print_config_game()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L94

## Description

Prints the game config.

[More Info on configs](../../documentation/configs)

## Signature

```c
void core_print_config_game(void);
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
```

## Examples

```c
void ft_on_tick(unsigned long tick)
{
	if (tick == 0)
	{
		core_print_config_game();
	}
// ...
```

## Related

- [Configs Documentation](../../documentation/configs)
