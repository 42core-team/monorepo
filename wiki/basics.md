The game is set on a square grid filled with objects.
- There can only ever be one object at one position, no more.
- Each object has a unique, unchanging id.

> [!WARNING]
> The memory location of objects may change between ticks. To store references to objects between ticks, it's recommended to use their id.

There is a central server that stores the game state. Each bot sends packets back and forth with the server.\
Once per tick, each client's local game state is updated.

> A tick is the smallest unit of measurement for time passing in Core Game. In each tick, all clients first receive the newest, up-to-date game state, then compute their next actions & send it back to the server. [More on ticks](https://www.reddit.com/r/explainlikeimfive/comments/4mn531/eli5_what_is_atick_in_gaming_development/).

Each tick, all game data you can see locally will be updated with the newest state information from the server.

> Both you and your opponent have all of the available information about the game to make exact decisions about, they know the same as you about the current game state.

To actually do something, you should use action functions.

> ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

Only action functions have an impact on the current game.\
The actual game state is saved on the server, and you can't modify it locally. Of course, you can locally change a unit's position struct to be right next to the enemy core on the client, but this won't be reflected in the results. ***You should be careful about this.*** Almost everything you see is just for reading, except for the action functions.

Each tick, use various functions to make an informed decision what to do. Perform an action using one of the action functions. The next tick, you will be able to see the effects of these changes.

> Don't be surprised if you move a unit, print its position immediately afterwards, and it hasn't changed yet—the next time your tick function gets called, it will have been updated.

The next tick, if an action function failed for some reason, a red "Action Failure" will be logged into the terminal. These are there to help you debug. \
They are almost always avoidable through doing the right checks in your code first. Keeping action failures to a minimum is highly recommended as it will prevent confusion if you ever encounter unintended behaviour.
