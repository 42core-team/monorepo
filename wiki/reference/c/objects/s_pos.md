---
title: "🧩 struct s_pos"
permalink: "s_pos"
sidebarTitle: "🧩 s_pos"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L27

## Description

Position structure for 2D coordinates.

Positions are 0-indexed, so position [0,0] and (assuming a gridSize of 20 in the config) [19, 19] are valid positions, but [-1,-1] and [20,20] are _not_.

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
int manhattan_distance(t_pos pos1, t_pos pos2)
{
	double x = (double)pos1.x - (double)pos2.x;
	double y = (double)pos1.y - (double)pos2.y;

	if (x < 0) x = -x;
	if (y < 0) y = -y;

	return ((int)(x + y));
}
```

## Related

- [Manhattan Distance Wikipedia](https://en.wikipedia.org/wiki/Taxicab_geometry)
