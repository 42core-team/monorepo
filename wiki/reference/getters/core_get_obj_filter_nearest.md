---
title: "⚙️ function core_get_obj_filter_nearest(...)"
permalink: "core_get_obj_filter_nearest"
sidebarTitle: "⚙️ get_obj_filter_nearest()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/get.c#L70

## Description

Get the nearest object to a given position matching a [custom filtering condition](documentation/getter_filtering).

[Manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry) is used to determine distance. If two objects or more have the same distance, either could be picked.

## Signature

```c
t_obj *core_get_obj_filter_nearest(t_pos pos, bool (*condition)(const t_obj *));
```

## Parameters

- `t_pos pos`: Position to search from
- `bool (*condition)(const t_obj *)`: Selection function pointer returning if the inputted object should be selected

## Return

- `t_obj *`: The nearest object that matches the condition or NULL if no such object exists or no condition is provided.

## Examples

```c
static bool is_deposit(const t_obj *obj)
{
	return (obj->type == OBJ_DEPOSIT);
}
t_obj *ft_get_deposit_nearest(t_pos pos)
{
	return core_get_obj_filter_nearest(pos, is_deposit);
}
// ...
t_obj *nearest_deposit = ft_get_deposit_nearest(miner->pos);
```

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
- [🧩 struct s_pos](reference/objects/s_pos)
- [Getter filtering Documentation](documentation/getter_filtering)
