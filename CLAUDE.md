
# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

CORE Game is a competitive programming platform where players write bots that control units on a 20x20 grid to destroy the opponent's core. The monorepo contains: a C++ game server, bot environments (C and TypeScript), client libraries for both languages, and a web-based replay visualizer.

## Architecture

```
Bots (C or TS) <--TCP/JSON--> Server (C++) ---> Replay JSON files ---> Visualizer (Web)
```

- **Server** (`server/`): C++17 game engine, single source of truth. Manages game state, validates actions, produces replay files. Built with CMake, uses nlohmann_json, json-schema-validator, xxhash.
- **Client Libraries**: Protocol abstraction layers that handle TCP connection, JSON serialization, and game state diffing.
  - C: `bots/c/client_lib/` — POSIX sockets, Makefile-built
  - TypeScript: `bots/ts/client_lib/` — Node.js `net` module, runs on Bun
- **Bots** (`bots/{c,ts}/{hardcore,softcore}/`): Player environments with two template bots each (`my-core-bot` and `gridmaster`). Configs at `bots/?/?/configs/`.
- **Visualizer** (`visualizer/`): Vite + vanilla TypeScript + CSS. Replays viewed at `localhost:4242` (dev) or `localhost:4000` (Docker).

## Build & Run Commands

### Full game (C bots, from devcontainer)
```bash
make LANGUAGE=c STAGE=dev DIFFICULTY=hardcore    # build everything and run a game
make LANGUAGE=c STAGE=prod DIFFICULTY=hardcore   # production build (optimized, no ASAN)
make stop                                        # kill running server and bots
make clean                                       # clean build artifacts
make fclean                                      # full clean including server binary
```

### Server only
```bash
cd server && make dev     # debug build with AddressSanitizer
cd server && make prod    # optimized -O3 build
cd server && make re      # full rebuild
```

Server binary runs as: `./server/server <server.config.json> <game.config.json> <data_dir> <player1_id> <player2_id>`

### TypeScript bots (from bot directory)
```bash
make run         # runs server + both bots (uses bun runtime)
make stop        # kill running processes
make devcontainer  # set up devcontainer with Devpod
```

### Visualizer
```bash
make visualizer   # or `make vis` — installs deps and starts vite dev server
# From visualizer directory:
npm run dev       # vite dev server
npm run build     # production build
npm run check     # biome lint + format
npm run format    # biome format only
npm run lint      # biome lint only
```

## Code Style

- **C++**: clang-format with LLVM base, tabs (width 4), Allman braces, 120-char columns, `-Wall -Wextra -Werror`
- **TypeScript/JS (visualizer)**: Biome formatter and linter. lint-staged runs `biome check --write` on commit.
- **Commits**: Prefix with affected areas: `[server|visualizer] Description`, `[client_lib] Fix bug`, `[wiki] Update docs`

## Key Conventions

- The root Makefile orchestrates building from within a devcontainer (paths like `/workspaces/monorepo/`)
- TS bot Makefiles assume running inside their Docker container (paths like `/core/server`)
- Game configs (`server.config.json`, `game.config.json`) define all game parameters (grid size, unit stats, costs, HP, damage)
- Replays are JSON files written to a `replays/` directory
- Git hooks are in `.githooks/` — set up via `git config core.hooksPath .githooks`
- GitHub Actions builds multi-arch Docker images (AMD64 + ARM64) to ghcr.io
