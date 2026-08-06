---
title: "⚙️ function core_get_units_byName_count(...)"
permalink: "core_get_units_byName_count"
sidebarTitle: "⚙️ get_units_byName_count()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Counts your units with the given name. Opposing units with the same name are excluded.

## Signature

```c
unsigned int core_get_units_byName_count(const char *name);
```

## Parameters

- `name`: Exact, case-sensitive name to count.

## Return

The number of your matching units, or `0` when the name is `NULL` or none of your units match.

## Example

```c
printf("Miner count: %u\n", core_get_units_byName_count("Miner"));
```

## Related

- [⚙️ function core_get_units_byName(...)](reference/c/getters/core_get_units_byName)
- [Unit Builder](documentation/unit_builder)
- [🧩 struct s_obj](reference/c/objects/s_obj)
