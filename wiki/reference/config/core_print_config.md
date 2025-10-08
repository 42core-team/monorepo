=====
HEADING="⚙️ function core_print_config(...)"
PERMALINK="core_print_config"
SIDEBAR_HEADING="⚙️ print_config()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L114

## Description

Prints the entire game config and all unit configs

## Signature

```c
void core_print_config(void);
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
		core_print_config();
	}
// ...
```


## Related
