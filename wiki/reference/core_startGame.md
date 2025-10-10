=====
HEADING="⚙️ function core_startGame(...)"
PERMALINK="startGame"
SIDEBAR_HEADING="⚙️ startGame()"
=====

## URL

https://github.com/42core-team/monorepo/blob/dev/client_lib/src/con_lib.c#L44

## Description

Starts the Core lib up, initializes the game, connects to the server & starts the game.

You only need to call this function once in your code to start up the Core library, and this line is already present in your code when you clone the repository. No need for you to anything. 💆‍♀️🧘‍♀️😎

## Signature

```c
int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug);
```

## Parameters

- `const char *team_name`: Name of your team
- `int argc`: Argument count from main function
- `char **argv`: Arguments from main function
- `void (*tick_callback)(unsigned long)`: Function that will be called every game tick once new server data is available.
- `bool debug`: Set to true to enable logging of all client/server packets sent & received. Largely used for Core Game Development, probably not useful in events.

## Return

- `int`: Returns 0 on success, another number on failure.

## Examples

```c
int main(int argc, char **argv)
{
	return core_startGame("FREDDATRON 6000 👾🤖💥", argc, argv, ft_on_tick, false);
}
```

## Related

/
