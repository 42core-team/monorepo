---
title: "⚙️ function core_get_objs_filter_count(...)"
permalink: "core_get_objs_filter_count"
sidebarTitle: "⚙️ get_objs_filter_count()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Get the count of objects that match a [custom filtering condition](documentation/getter_filtering).

## Signature

```c
unsigned int core_get_objs_filter_count(bool (*condition)(const t_obj *));
```

## Parameters

- `bool (*condition)(const t_obj *)`: Function returning whether an object should be counted. Pass `NULL` to count every object.

## Return

- `unsigned int`: Count of objects in the game that match the condition.

## Examples

```c
// #include <string.h>
bool ft_is_warrior(const t_obj *obj)
{
	return (obj->type == OBJ_UNIT && obj->s_unit.name
		&& strcmp(obj->s_unit.name, "Warrior") == 0);
}
// ...
printf("Warrior Count: %u\n", core_get_objs_filter_count(ft_is_warrior));

```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
- [Getter filtering Documentation](documentation/getter_filtering)
