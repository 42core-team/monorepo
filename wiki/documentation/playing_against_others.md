## Locally

### Locally with executables

Have the person you want to play against send them their executable. In their cloned git repo, it will be created under `my-core-bot/bot`.

<pre>
/core/server /workspace/configs/server.config.json /workspace/configs/game.config.json /core/data 42 43 > /dev/null &
./PATH_TO_PLAYER_1_EXECUTABLE 42 > /dev/null &
./PATH_TO_PLAYER_2_EXECUTABLE 43
</pre>

Running the following code will start a game of the player 1 executable against the player 2 executable.

### Locally with source files

Alternatively, you can play against others if you have their full code folder (their `my-core-bot/` directory) available by manually specifying the two bots that are playing. To do this, change the `PLAYER1_BOT` and `PLAYER2_BOT` variables to point to another players copied over source files: `player_source_files_folder/player_executable_name`.

<pre>
make PLAYER1_BOT=my-core-bot/bot PLAYER2_BOT=gridmaster/gridmaster
</pre>

You will most likely want to keep `PLAYER1_BOT` pointing to `my-core-bot/bot`, which will make your own bot participate in the fight.\
You can even use this method to create your own custom testing opponent or to play against yourself and see what happens.\
Be aware the bot in the my-core-bot directly will get used in the public fights on the website.


## Online

Once you have pushed some code, you can compete on the website in the queue or tournament.

The queue will pit random people that join the queue at a similar time against each other.\
The tournament will at the end determine the winner of your event.

Using the queue often is a great idea to see how good your bot is in what aspects of Core Game.

If your code runs locally but not on the website, you may want to have a look at the [website setup guide](documentation/website_setup).
