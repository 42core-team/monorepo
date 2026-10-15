---
title: "⚙️ function Move(...)"
permalink: "go_Move"
sidebarTitle: "⚙️ Move()"
---

```go
func Move(unit *game.Object, pos game.Position) error
```

Queues a move to an adjacent empty position. Returns an error for an unavailable client, nil/non-unit actor, or out-of-bounds destination. Use `errors.Is` with `core.ErrClientUnavailable`, `game.ErrNilObject`, `game.ErrWrongObjectType`, or `core.ErrInvalidPosition`.

A nil error means queued locally, not executed. Cooldown, ownership, range, and occupancy are validated by the server. Server error messages are printed by the library.

```go
if err := core.Move(unit, game.Position{X: unit.Pos.X + 1, Y: unit.Pos.Y}); err != nil {
	log.Printf("move: %v", err)
}
```

Use [`Travel`](reference/go/actions/Travel) for multi-tick routing.
