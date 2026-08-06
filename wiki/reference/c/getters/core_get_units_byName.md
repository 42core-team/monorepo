---
title: "⚙️ function core_get_units_byName(...)"
permalink: "core_get_units_byName"
sidebarTitle: "⚙️ get_units_byName()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Gets each of your units with the given name. Opposing units with the same name are excluded.

Names are useful for finding a unit design or role now that units are built from components instead of fixed unit types.

## Signature

```c
t_obj **core_get_units_byName(const char *name);
```

## Parameters

- `name`: Exact, case-sensitive name to find.

## Return

A `NULL`-terminated array of your matching units, or `NULL` when the name is `NULL` or none of your units match.

Free the returned array. Do not free the objects inside it.

## Example

```c
t_obj **miners = core_get_units_byName("Miner");

for (size_t i = 0; miners && miners[i]; i++)
{
	// Run this team's mining logic.
}

free(miners);
```

## Related

- [⚙️ function core_get_units_byName_count(...)](reference/c/getters/core_get_units_byName_count)
- [Unit Builder](documentation/unit_builder)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [⚙️ function core_get_objs_filter(...)](reference/c/getters/core_get_objs_filter)
