# Game Struct

## `t_game`

Contains all the data about the game. Read it to your liking! Access it anywhere by typing `game.`

- `elapsed_ticks`: The elapsed ticks since the game started. Same as what you're passed in your tick callback function.
- `config`: The config contains base informations about the game that don't change such as the map size and the unit types.
- `my_team_id`: The id of the team that you are playing for. Saved in the team_id field of your cores and units.
- `objects`: List of all objects (units, cores, deposits, gem piles, bombs, walls etc.) and their informations. NULL-terminated.

```c
typedef struct s_game
{
	unsigned long elapsed_ticks;
	t_config config;
	unsigned long my_team_id;
	t_obj **objects;
} t_game;
```

## `game`

This variable contains all the data about the game.
It gets updated every time your function is called.

```c
extern t_game game;
```

## `core_startGame`

Starts the connection lib up, initializes the game, connects to the server & starts the game.

- `team_name`: Name of your team
- `argc`: Argument count from main function
- `argv`: Arguments from main function
- `tick_callback`: Function that will be called every game tick once new server data is available.
- `debug`: Set to true to enable logging of all client/server packets sent & received.
- `return`: 0 on success, another number on failure.

```c
int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug);
```

> **NOTE**: Do not free the central objects array or the objects themselves. They are fully handled by the library and freeing anything you're not supposed to will probably mess up your bot entirely. Functions that give you the responsibility to free the result when you call them are explicitly marked in this wiki.
