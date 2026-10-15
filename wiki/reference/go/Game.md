---
title: "🧩 Global Game state"
permalink: "go_Game"
sidebarTitle: "🧩 Game"
---

`core.Game` is the global current state, like `game` in the C library. The
library initializes it in `core.StartGame` and updates it before each callback.
You never need to create a game object or pass it through your helpers.

```go
// Exported by package core:
var Game *game.Game
```

The state has `Config` (`game.Config`), `Objects` (`[]*game.Object`),
`ElapsedTicks` (`uint`), and `MyTeamID` (`uint`) fields. Entity types such as
`game.Object` and `game.Position` still come from the `/game` package.

`Config` is fixed game configuration. `Objects` contains the current cores, units, deposits, walls, and gem piles. State diffs update the existing objects and remove objects reported as dead.

Treat state and objects as read-only and access them only from the tick callback.
Keep object IDs rather than pointers between ticks. A later `StartGame` replaces
the state; do not cache the global pointer across games.

Server action error messages are printed by the library and do not terminate `StartGame`; transport and protocol errors do.

```go
func tick(elapsedTicks uint) {
	fmt.Printf("tick %d, objects %d\n", elapsedTicks, len(core.Game.Objects))
}
```

See [StartGame](reference/go/StartGame), [game getters](reference/go/getters/GameGetters), [Object](reference/go/objects/Object), and [Config](reference/go/config/Config).
