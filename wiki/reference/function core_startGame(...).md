## Description

Starts the Core lib up, initializes the game, connects to the server & starts the game.

You only need to call this function once in your code to start up the connection lib, and this line is already present in your code when you clone the repository. No need for you to anything. 💆‍♀️🧘‍♀️😎

## Signature

```c
int core_startGame(const char *team_name, int argc, char **argv, void (*tick_callback)(unsigned long), bool debug);
```

## Parameters

- `team_name`: Name of your team
- `argc`: Argument count from main function
- `argv`: Arguments from main function
- `tick_callback`: Function that will be called every game tick once new server data is available.
- `debug`: Set to true to enable logging of all client/server packets sent & received.
- `return`: 0 on success, another number on failure.

## Examples

```c
int main(int argc, char **argv)
{
	return core_startGame("FREDDATRON 6000 👾🤖💥", argc, argv, ft_on_tick, false);
}
```

## Related
