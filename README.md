# CORE GAME Monorepo

## How to Run

1. Clone this via SSH
2. Open this via [Microsoft devcontainer extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) in VSCode
3. Run `make` to build, `make vis` to start the visualizer.
	- `make hp` -> Run the game with the hardcore config on prod stage;
	- `make sp` -> Run the game with the softcore config on prod stage;
	- `make hd` -> Run the game with the hardcore config on dev stage;
	- `make sd` -> Run the game with the softcore config on dev stage;

---

## Folders & Project Structure

- bots/?/client_lib/ -> previously connection, the helper lib used by participants
- server/ -> previously even_COREnier, the gameplay server, single source of truth connection players during matches, cpp
- visualizer/ -> previously debug_visualizerm displays replay files generated from the server with vanilla html ts css
- bots/ -> bot coding environment for Core players. Put files or folders into the .coreignore to not have them show up in the generated player repositories.
- wiki/ -> previously it's own repo, handles all the documentation. Deployed under wiki.coregame.sh; Please make sure to also update the wiki when something wiki-relevant changes.

---

## How to write a bot

- configs are at bots/?/?/configs
- to write a test bot, write it in bots/?/?/your_bot
- you can also add a new folder for your bot if and add the folder to the coreignore, this will stop it from being added to the user repos
- please be careful about not keeping powerful bots public and making the events uninteresting

---

## Issues

- Label the issue with all the areas of the repo that it relates to. Issue Label names should exactly mirror the names of the respective directoris in the repo.
- Give all issues a clear type.

---

## Commit naming structure

- Mirroring the way issues are labeled, we'll start all commits with `[...]` and putting in there the areas the commit is impacting, mirroring label names or the folder names

=> `[server|visualizer] Replay format move action contract tweaks`
=> `[client_lib] core_util_distance() math error bugfix`
=> `[wiki] Fix some spelling mistakes`
