---
title: "🧩 struct s_pos"
permalink: "s_pos"
sidebarTitle: "🧩 s_pos"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L27

## Description

Position structure for 2D coordinates.

Positions start at 0. Both coordinates must be smaller than `game.grid_size`. On a 20×20 grid, [0,0] and [19,19] are valid, but [20,20] is not.

There can never be two objects at the same position. To check whether there is something at a given position, use `core_get_obj_from_pos()`. If it returns `NULL`, there's nothing there.

## Signature

```c
typedef struct s_pos
{
	unsigned short x;
	unsigned short y;
} t_pos;
```

## Parameters

- `unsigned short x`: X coordinate
- `unsigned short y`: Y coordinate

## Examples

```c
#include <stdlib.h>

int manhattan_distance(t_pos pos1, t_pos pos2)
{
	return abs((int)pos1.x - (int)pos2.x) + abs((int)pos1.y - (int)pos2.y);
}
```

## Related

- [Manhattan Distance Wikipedia](https://en.wikipedia.org/wiki/Taxicab_geometry)
