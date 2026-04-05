---
title: "🧩 struct s_game"
permalink: "s_game"
sidebarTitle: "🧩 s_game"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/c/client_lib/inc/core_lib.h#L166

## Description

The game struct is your primary source of information about the happenings in the game. It contains the entire game state.

The game struct gets instantiated for you by the Core Libary when you call core_startGame. Read it to your liking! Access it anywhere by typing `game.` in any files that include `core_lib.h`.

## Signature

```c
typedef struct s_game
{
	unsigned long elapsed_ticks;
	t_config config;
	unsigned long my_team_id;
	t_obj **objects;
} t_game;
```

## Parameters

- `unsigned long elapsed_ticks`: The elapsed ticks since the game started. Same as what you're passed in your tick callback function.
- `t_config config`: The config contains base information about the game that don't change such as the map size and the unit types.
- `unsigned long my_team_id`: The id of the team that you are playing for. Saved in the team_id field of your cores and units.
- `t_obj **objects`: List of all objects (units, cores, deposits, gem piles, bombs, walls etc.) and their informations. NULL-terminated.

## Examples

```c
for (int i = 0; game.objects && game.objects[i]; i++)
{
	t_obj *obj = game.objects[i];
	// do something with the object here
}
```

## Related

- [🧩 struct s_config](reference/c/config/s_config)
- [🧩 struct s_obj](reference/c/objects/s_obj)
