## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L36

## Description

Position structure for 2D coordinates.

> Positions are 0-indexed, so position [0,0] and (assuming a gridSize of 20 in the config) [19, 19] are valid positions, but [-1,-1] and [20,20] are *not*.

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

## Related
