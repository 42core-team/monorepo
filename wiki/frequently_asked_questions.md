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

Other questions? Let us know and this section will get longer!
