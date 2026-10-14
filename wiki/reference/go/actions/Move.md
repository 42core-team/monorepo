---
title: "⚙️ method Bot.Move(...)"
permalink: "go_Move"
sidebarTitle: "⚙️ Move()"
---

```go
func (b *Bot) Move(unit *game.Object, pos game.Position) error
```

Queues a move to an adjacent empty position. Returns an error for an unavailable client, nil/non-unit actor, or out-of-bounds destination. Use `errors.Is` with `coregame.ErrClientUnavailable`, `game.ErrNilObject`, `game.ErrWrongObjectType`, or `coregame.ErrInvalidPosition`.

A nil error means queued locally, not executed. Cooldown, ownership, range, and occupancy are validated by the server. Later rejections appear in `g.ActionRejections`.

```go
if err := bot.Move(unit, game.Position{X: unit.Pos.X + 1, Y: unit.Pos.Y}); err != nil {
	log.Printf("move: %v", err)
}
```

Use [`Bot.Travel`](reference/go/actions/Travel) for multi-tick routing.
