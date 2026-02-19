---
title: "⚙️ function core_get_obj_from_pos(...)"
permalink: "core_get_obj_from_pos"
sidebarTitle: "⚙️ get_obj_from_pos()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/src/public/get.c#L12

## Description

Get any object based on its position.

## Signature

```c
t_obj *core_get_obj_from_pos(t_pos pos);
```

## Parameters

- `t_pos pos`: The position of the object the function should get

## Return

- `t_obj *`: The object at the position or NULL if no such object exists.

## Examples

```c
bool ft_is_pos_empty(t_pos pos)
{
	return core_get_obj_from_pos(pos) == NULL;
}
```

## Related

- [🧩 struct s_obj](reference/objects/s_obj)
- [🧩 struct s_pos](reference/objects/s_pos)
