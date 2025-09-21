# 🏠 The Art of CORE

## 🤗 Welcome to the Official CORE Documentation 📚

Read it carefully, there are many helpful tidbits hidden in here.

Especially the tips & tricks section down below and the tip boxes in the Core Library Pages can be incredibly helpful.

Reading the documentation sucks, but it was handwritten to be as helpful as possible and is worth a skim. If there was a question that the documentation didn't answer, please let us know so we can tell you and improve the documentation.

# 👾 What is CORE?

CORE GAME is a programming competition where you can write a bot and compete against the bots made by other people. Each of you has a base on the map called your core, which is the place you need to protect as if it is destroyed you lose.

You don't have any direct access to the game while its playing, like in a classic game. Instead you create a program that executes your strategy and dynamically adapts to any situation thrown at it. Later, your bot can play against the bots the other event participants wrote.

To avoid being defeated, you can spawn a variety of units to defend and attack your opponent. These all have different stats, abilities and drawbacks. To spawn them, you need gems. To have a look at what units are available, look at the [config](4_configs).

You can receive gems by having your units attack the gem deposits on the map or via idle income which you get automatically. Good resource management is critical to creating a successful bot.

# 📜 Let's exp**LORE** the **CORE** **LORE**

> This is required reading and you will immediately lose Core Game if you don't memorize the following text.

In the year 2542, as the burned remains of World War III ushered in
a fragile new beginning, Earth rose once more from the ashes of a
nuclear wasteland. Hidden aristocrats emerged from their secret
bunkers, resplendent in opulence, to reclaim a land both scarred
and grim.
Across this ravaged realm, humanity had been twisted into ghastly
forms: ghostly skeletons wandered like lost souls, while ugly green
goblins - reminders of what once was - stalked the barren wilds.

Driven by a perverse desire for luxury and brutal entertainment,
these aristocrats built vast arenas of horror. Within these
bloodstained colosseums, condemned creatures were forced to engage
in a savage dance for survival, their suffering displayed as a
repulsive show for those who delight in cruelty.

Dear Strategist, this letter is not a request, it's an order - a
call to arms on the eve of battle. Your destiny, and indeed your
very life, are bound to the fate of these warriors.

The time has come to gather your forces with precision and courage,
turning every command into a stroke of genius. Now, as you stand on
the brink of combat, let your resolve shine like a guiding light.

Embrace the challenge and let your code strike true like a finely
honed blade.

Onward - to glory! ⚔️

# 🚀 Getting Started with Your Own CORE Bot

To start building your own CORE bot, ensure you've completed the following prerequisites:

### Prerequisites

1. **Be a Registered CORE Participant/Team** ✨
	- Visit our website, [COREGAME.de](https://coregame.de/), sign up, and add your team members.

2. **Clone Your CORE Repository** 📧
	- You will find an invite link to your dedicated CORE repository on GitHub on your teams page.

3. **Follow the README in the repository** 📧
	- Basically, run `make devcontainer`, and then you can get started! Compile using `make`, visualizer to view the games is under `localhost:4000`, everything in `src/` will be compiled into your bot.

🎉 **You are now ready to start coding!** 😎

4. **Push some code!** 🔝
	- The code you pushed will be used to execute games in the queue and in the final tournament. So make sure to push your code regularly and use to queue to test your bot against other participants!

### Play and test against other Teams 🎮

Once the default test bot is too boring and you always win, feel free to share your compiled bot with other teams and play against them. You can do this via the queue feature on the website or locally. Of course, you can't force them but it might benefit both of you to see your bots in _real_ action.

## 📝 Example Code

In the `my-core-bot/src/` folder there will already be a little example bot that does basic actions.

# 🔎 On the Core Game Library

- A tick is the smallest unit of measurement for time passing in Core Game. In each tick, all clients first receive the newest, up-to-date game state, then compute their next actions & send it back to the server.
- You will have the information in the game struct updated between ticks. All game data you can see locally will be updated with the newest state information from the game server. Both you and your opponent have all of the available information about the game to make exact decisions about, they know the same as you about the current game state.
- You will be able to change things about the game state by executing actions. Changing fields locally won't make a difference.
- Be careful how you interact with the game! The actual gamestate is saved in the server, and you can't modify it locally. Of course you can change a units position struct to be right next to the enemy core locally, but this won't be reflected in the results. Instead, all game interactions must be done via the `core_action_*` functions, and all other functions and structs are just for you to form an informed decision about your next move.
- When you execute an action, the result will only be reflected in the game's next tick. Don't be surprised therefore if you move a unit, print it's position immediately afterwards and it hasn't changed yet - the next time your tick function gets called, it will have been updated.

# 🔥 Tips & Tricks

> Look at the config! Knowing the exact damage values is exactly the kind of thing you'd want to consider before trying out a strategy. And you don't want to be finding out there were two more units you didn't even know about two hours before the event ends!

> Be careful about attacking Ghosts! Object could be uninitialized or dead, so take care not to start your full assault on a unit thats already ascended into the afterlife. Check an objects state to avoid this.

> Consider the powerful possibilities of the `void *data` field in every object! Here, you can save any data you want, allowing you to easily execute more detailed strategies and coordinate your troops efficiently! The library wont touch it. Just remember to free everything at the end.

> If you can't find a standard library function to do what you're looking for, don't sweat it! Everything there is to know about the game at the current moment can be found in `the game struct`. Get any info you want yourself!

> Be careful what you free! Some standard library functions need you to free their returned array, some don't! You never need to free anything in the game struct, and you never need to free a single `t_obj *` you haven't manually allocated yourself! If you free things wrong, the library and subsequently your bot will fail pretty spectacularly pretty quickly.

> Looking at the [taxicab / manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry) might be useful. It's what the entire project uses. Who like Pythagoras and euclidian geometry anyways? Diamonds for the win!

---

# ❓ Frequently Asked Questions (FAQ)

### 💃 Do I need to adhere to 42 Norminette?
> No! *Dobby is free!*

### 🔧 Where can I see the implementations of the standard library? Where can I find the core_lib.h file?
> Check out [our GitHub](https://github.com/42core-team) to see all of the CORE source code - it may give you an easy starting point for some functionalities.\
> The core_lib.h file can be found on our [GitHub](https://github.com/42core-team/monorepo/blob/dev/client_lib/inc/core_lib.h) (careful, the latest version might not match the Core Version you are playing on. Look at the releases.), and in your filesystem will be available under `/core/inc`, which can probably most easily found by cmd+clicking onto the include in the `both.h` header. The easiest way to loop at it though is to not look at it and instead look at the wiki, which contains all the relevant info and more.

### 🐞 What should I do if my bot isn't responding / starting up?
> First, make sure the **Docker container** is running properly, that you've cloned with SSH and that you're able to see logs from your bot in the terminal. Check for any errors in the console or in your logs. Double-check the user loop logic to ensure it’s being executed as expected. Quit the devcontainer and reopen it using the `make devcontainer` command.

### 🔍 It seems I am missing some files. Where are they?
> We have hidden some files when you open up your core bot repository in Visual Studio Code. These are not useful to you unless you want to start tinkering around a bit, so don't worry about us hiding critical info. We did this so nobody gets confused. If you want to disable it, look at the repository outside of VSCode or via the terminal, go into `.vscode/settings.json` and remove every file you don't want hidden from the exclude list.
>
> If that doesn't help you, you probably accidentally deleted something. Well, it's a git repository for a reason.

### 📚 Are there any limitations to what I can add to the bot?
> Your bot will be run in a separate Docker container from the server and other bots, and this container won't have an internet connection.
>
> See also: [Running on the Website](7_website)

### 🪻 Are there limits to how many gems units or cores can hold?
> No. Go nuts.

### 🧐 I have found a bug! What do I do?

> 👀

> (Please tell us and open a [GitHub issue](https://github.com/42core-team/monorepo/issues).)

##### Other questions? Let us know and this section will get longer!

# 🏆 To win the event

> Don't overcomplicate things! Simple but well-balanced bots may often reign over overcomplicated but ineffective bots.

> Play against other players as often as possible!

> Have fun! Don't take things too seriously & be nice to each other. Embrace the good vibes in a friendly competition! ⚡️

Good luck from the CORE team! 🚀🎸🔥💥🗿
