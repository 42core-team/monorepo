---
title: "⚙️ function core_print_objs(...)"
permalink: "print_objs"
sidebarTitle: "⚙️ print_objs()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/public/printing.c#L49

## Description

Prints multiple objects.

## Signature

```c
t_obj **core_print_objs(t_obj **objs);
```

## Parameters

- `t_obj **objs`: The objects to print information about.

## Return

- `t_obj **`: Returns the inputted objects array, so you can easily free in the same line as you print. (see example)

### stdout

```
---ID: 160
Type: Unit
Position: (8, 7)
HP: 5
Unit Type: 0
Team ID: 43
gems: 0
Action Cooldown: 0
---
---ID: 168
Type: Unit
Position: (1, 2)
HP: 35
Unit Type: 0
Team ID: 43
gems: 0
Action Cooldown: 4
---
...
```

## Examples

```c
free(core_print_objs(core_get_objs_filter(ft_is_deposit)));
```

This example will get an array of all deposits using the getter system, print it, then free it, all in the same line. _Awesome!_

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
