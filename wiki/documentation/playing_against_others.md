## Locally

Have the person you want to play against send them their executable. In their cloned git repo, it will be created under `my-core-bot/bot`.

```bash
/core/server /workspace/configs/server.config.json /workspace/configs/game.config.json /core/data 42 43 > /dev/null &
./PATH_TO_PLAYER_1_EXECUTABLE 42 > /dev/null &
./PATH_TO_PLAYER_2_EXECUTABLE 43
```

Running the following code will start a game of the player 1 executable against the player 2 executable.

## Online

Once you have pushed some code, you can compete on the website in the queue or tournament.

The queue will pit random people that join the queue at a similar time against each other.\
The tournament will at the end determine the winner of your event.

Using the queue often is a great idea to see how good your bot is in what aspects of Core Game.

If your code runs locally but not on the website, you may want to have a look at the [website setup guide](documentation/website_setup).
