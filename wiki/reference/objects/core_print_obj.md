---
title: "⚙️ function core_print_obj(...)"
permalink: "print_obj"
sidebarTitle: "⚙️ print_obj()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/printing.c#L5

## Description

Prints all information about the current game state of a given object.

## Signature

```c
void core_print_obj(t_obj *obj);
```

## Parameters

- `t_obj *obj`: The object to print information about

## Return

void

### stdout

```
---ID: 188
Type: Unit
Position: (1, 2)
HP: 23
Unit Type: 0
Team ID: 43
gems: 0
Action Cooldown: 3
```

## Examples

```c
// move units
t_obj **units = ft_get_units_own();
for (int i = 0; units && units[i]; i++)
{
	t_obj *obj = units[i];

	printf("Moving the next object!\n");
	core_print_obj(obj);
```

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
