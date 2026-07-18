---
title: "⚙️ function core_get_units_by_name(...)"
permalink: "core_get_units_by_name"
sidebarTitle: "⚙️ get_units_by_name()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L193

## Description

Gets every unit with the given name. It searches units from all teams.

Names are useful for finding a unit design or role now that units are built from components instead of fixed unit types.

## Signature

```c
t_obj **core_get_units_by_name(const char *name);
```

## Parameters

- `name`: Exact, case-sensitive name to find.

## Return

A `NULL`-terminated array of matching units, or `NULL` when the name is `NULL` or no unit matches.

Free the returned array. Do not free the objects inside it.

## Example

```c
t_obj **miners = core_get_units_by_name("Miner");

for (size_t i = 0; miners && miners[i]; i++)
{
	if (miners[i]->s_unit.team_id == game.my_team_id)
	{
		// Run this team's mining logic.
	}
}

free(miners);
```

## Related

- [Unit Builder](documentation/unit_builder)
- [🧩 struct s_obj](reference/c/objects/s_obj)
- [⚙️ function core_get_objs_filter(...)](reference/c/getters/core_get_objs_filter)
