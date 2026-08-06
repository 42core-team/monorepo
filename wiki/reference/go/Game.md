---
title: "🧩 type Game"
permalink: "go_Game"
sidebarTitle: "🧩 Game"
---

`Game` is the state snapshot passed to every tick callback.

```go
type Game struct {
	Config       Config
	Objects      []*Object
	ElapsedTicks uint
	MyTeamID     uint
}
```

`Config` is fixed game configuration. `Objects` contains the current cores, units, deposits, walls, and gem piles. State diffs update the existing objects and remove objects reported as dead.

```go
func tick(g *game.Game, bot *coregame.Bot) {
	fmt.Printf("tick %d, objects %d\n", g.ElapsedTicks, len(g.Objects))
}
```

See [game getters](reference/go/getters/GameGetters), [Object](reference/go/objects/Object), and [Config](reference/go/config/Config).
