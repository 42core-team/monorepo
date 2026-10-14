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
	ActionRejections []ActionRejection
}
```

`Config` is fixed game configuration. `Objects` contains the current cores, units, deposits, walls, and gem piles. State diffs update the existing objects and remove objects reported as dead.

`ActionRejections` contains server rejections received with this tick and is cleared on the next update. Each value implements `error` and has `Tick` and `Message` fields. The protocol does not supply action IDs or stable error codes. Rejections do not terminate `Run`; transport and protocol errors do.

```go
func tick(g *game.Game, bot *coregame.Bot) {
	fmt.Printf("tick %d, objects %d\n", g.ElapsedTicks, len(g.Objects))
	for _, rejection := range g.ActionRejections {
		log.Printf("tick %d: server rejected action: %v", rejection.Tick, rejection)
	}
}
```

See [game getters](reference/go/getters/GameGetters), [Object](reference/go/objects/Object), and [Config](reference/go/config/Config).
