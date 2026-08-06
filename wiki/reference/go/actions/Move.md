---
title: "⚙️ method Bot.Move(...)"
permalink: "go_Move"
sidebarTitle: "⚙️ Move()"
---

```go
func (b *Bot) Move(unit *game.Object, pos game.Position)
```

Queues a move to an adjacent empty position. The server rejects invalid positions, non-unit sources, and units with a positive action cooldown. A nil unit queues nothing.

```go
bot.Move(unit, game.Position{X: unit.Pos.X + 1, Y: unit.Pos.Y})
```

Use [`Bot.Travel`](reference/go/actions/Travel) for multi-tick routing.
