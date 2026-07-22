---
title: "⚙️ function core_get_objs_filter(...)"
permalink: "core_get_objs_filter"
sidebarTitle: "⚙️ get_objs_filter()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/get.c

## Description

Get all objects matching a [custom filtering condition](documentation/getter_filtering).

> [!WARNING]
> You are responsible for **freeing the array** returned by this function, but not the objects inside it. They are the same object instances as those in the `game.objects` array.

> [!WARNING]
> Filter conditions must be deterministic, side-effect-free, must not mutate/free objects, and should return the same result throughout the call.

## Signature

```c
t_obj **core_get_objs_filter(bool (*condition)(const t_obj *));
```

## Parameters

- `bool (*condition)(const t_obj *)`: Selection function pointer returning whether the input object should be selected

## Return

- `t_obj **`: Null-terminated array of selected objects or NULL if no condition is provided or no objects match the condition. **You are responsible to free the returned array.**

## Examples

```c
t_obj **warriors = core_get_objs_filter(ft_is_warrior);
for (int i = 0; warriors && warriors[i]; i++)
{
	// warrior move & attack logic
}
free(warriors);
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
- [Getter filtering Documentation](documentation/getter_filtering)
