were on a grid of objects, there are never two objects at one position
each object has a unique id
there are client and server communicating, changing random client state stuff doesnt mean its reflected in the server

// TODO


- A tick is the smallest unit of measurement for time passing in Core Game. In each tick, all clients first receive the newest, up-to-date game state, then compute their next actions & send it back to the server.
- You will have the information in the game struct updated between ticks. All game data you can see locally will be updated with the newest state information from the game server. Both you and your opponent have all of the available information about the game to make exact decisions about, they know the same as you about the current game state.
- You will be able to change things about the game state by executing actions. Changing fields locally won't make a difference.
- Be careful how you interact with the game! The actual gamestate is saved in the server, and you can't modify it locally. Of course you can change a units position struct to be right next to the enemy core locally, but this won't be reflected in the results. Instead, all game interactions must be done via the `core_action_*` functions, and all other functions and structs are just for you to form an informed decision about your next move.
- When you execute an action, the result will only be reflected in the game's next tick. Don't be surprised therefore if you move a unit, print it's position immediately afterwards and it hasn't changed yet - the next time your tick function gets called, it will have been updated.





> ACTION FUNCTIONS are used to perform actions in the game, like creating units, moving them, attacking, etc. Their changes are applied between ticks.

> ACTION FUNCTIONS are the only way to interact with the game state. There is no point in setting the values of object structs locally, as they won't have an effect on the actual gamestate, only actions will.

If you are wondering why a certain action is failing, look through [its code in the server](https://github.com/42core-team/monorepo/tree/dev/server/src/action)! You can see everything that is validated about the action before it is executed there.

All actions from all teams are put into a list which order gets randomized before any actions get executed. That means if two actions conflict each other, a random decision is made, and everything is always 100% fair.




GETTER FUNCTIONS are used to get information about the current game state. They are **highly useful** and it is suggested to utilize them, though if you don't want to you can just loop over all the data in `game.objects`.

> **TIP**: If you are unsure about how to use the getter functions and the function filtering pointers, you are provided with a `getter.c` file that uses them and provides some basic functions with them. Have a look to figure things out.