---
title: "⚙️ method Bot.Move(...)"
permalink: "go_Move"
sidebarTitle: "⚙️ Move()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Moves a unit to a specific position.

- Units can only move one tile up, down, left or right; for more see [Action Position Limits](documentation/action_position_limits).
- Units can only move if their action cooldown is 0, for more see [Cooldowns](documentation/cooldowns).

## Signature

```go
func (b *Bot) Move(unit *game.Object, pos game.Position)
```

## Parameters

- `unit *game.Object`: The unit that should move
- `pos game.Position`: The position where the unit should move to

## Return

void (nil-safe: does nothing if unit is nil)

## Examples

```go
target := getTarget(g)
unit := getMovingUnit(g)

if unit.Pos.X < target.Pos.X {
	b.Move(unit, game.Position{X: unit.Pos.X + 1, Y: unit.Pos.Y})
}
if unit.Pos.Y < target.Pos.Y {
	b.Move(unit, game.Position{X: unit.Pos.X, Y: unit.Pos.Y + 1})
}
// ...
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct Position](reference/go/objects/Position)
