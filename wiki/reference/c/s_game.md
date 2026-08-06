---
title: "🧩 struct s_game"
permalink: "s_game"
sidebarTitle: "🧩 s_game"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h

## Description

Contains the game state available to your bot. Use the global `game` variable from any file that includes `core_lib.h`.

The values are updated before every tick callback.

## Signature

```c
typedef struct s_game
{
	unsigned long elapsed_ticks;
	unsigned long my_team_id;
	unsigned short grid_size;
	t_obj **objects;
} t_game;
```

## Fields

- `elapsed_ticks`: Ticks since the game started. This is the same value passed to your tick callback.
- `my_team_id`: ID of your team. Your core and units use the same ID.
- `grid_size`: Width and height of the square grid.
- `objects`: `NULL`-terminated array of every current object.

The library owns the objects and the array. Do not free `game.objects`.

## Example

```c
for (size_t i = 0; game.objects && game.objects[i]; i++)
{
	t_obj *obj = game.objects[i];
	// Read the object here.
}
```

## Related

- [🧩 struct s_obj](reference/c/objects/s_obj)
