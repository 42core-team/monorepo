## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L98

## Description

Prints the game config.

## Signature

```c
void core_print_config_game(void);
```

## Parameters

void

## Return

void

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
