The game struct is your primary source of information about the happenings in the game

> **WARNING**: Do not free the central objects array or the objects themselves. They are managed by the library and freeing anything you're not supposed to will probably mess up your bot entirely. Functions that give you the responsibility to free the result when you call them are explicitly marked in this wiki with a big warning box.

> => IF YOU DIDN'T ALLOCATE IT, DON'T FREE IT.

# Client Lib Functions & Structs

## struct [`t_game`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L177)

Contains all the data about the game. Read it to your liking! Access it anywhere by typing `game.`

- `elapsed_ticks`: The elapsed ticks since the game started. Same as what you're passed in your tick callback function.
- `config`: The config contains base information about the game that don't change such as the map size and the unit types.
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

---

## variable [`game`](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h#L201)

This variable contains all the data about the game.
It is the library-provided instance of the t_game struct.
Include `core_lib.h` in every file you want to use it.

```c
extern t_game game;
```

**Usage Example (Game)**

```c
for (int i = 0; game.objects && game.objects[i]; i++)
{
	t_obj *obj = game.objects[i];
	// do something with the object here
}
```

---

## function [`core_startGame`](https://github.com/42core-team/monorepo/blob/dev/client_lib/src/con_lib.c#L44)

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
