# CORE GAME Monorepo

## How to Run

1. Clone this via SSH
2. Run make update to initialize the my-core-bot submodule
3. Open this via Microsoft devcontainer extension in VSCode
4. Run make to build, make run to run, make vis to start the visualizer, and make debug to run with the outputs of server and the other bot, not just yours.

---

## Folders & Project Structure

- client_lib/ -> previously connection, the C helper lib used by participants
- server/ -> previously even_COREnier, the gameplay server, single source of truth connection players during matches, cpp
- visualizer/ -> previously debug_visualizerm displays replay files generated from the server with vanilla html ts css
- my-core-bot/ -> bot coding environment for Core players, containing two bots. Imported here as a submodule because players must receive a fork of my-core-bot only during events. This is included here so monorepo can also work with the same bots and configs as my-core-bot without us having duplicate code and copying files around.

---

## How to write a bot

- configs are at my-core-bot/configs
- to write a test bot, write it in my-core-bot/your_bot

---

## Issues

- Label the issue with all the areas of the repo that it relates to.
- Give all issues a clear type.

--- 

## Branches

- my-core-bot branches are 100% synced with this repos branches. If a monorepo branch gets deleted or merged, do the same for the my-core-bot branch.

---

## Commit naming structure

- Mirroring the way issues are labeled, we'll start all commits with `[...]` and putting in there the areas the commit is impacting, mirroring label names or the folder names, e.g. `[server|visualizer] Replay format move action contract tweaks`
