---
title: "🧩 struct Position"
permalink: "go_Position"
sidebarTitle: "🧩 Position"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/types.go

## Description

Position structure for 2D coordinates.

Positions are 0-indexed, so position [0,0] and (assuming a gridSize of 20 in the config) [19, 19] are valid positions, but [-1,-1] and [20,20] are _not_.

There can never be two objects at the same position. To check whether there is something at a given position, use `g.ObjectAtPos()`. If it returns `nil`, there's nothing there.

## Signature

```go
type Position struct {
	X uint
	Y uint
}

func NewPosition(x, y uint) Position
```

## Parameters

- `X uint`: X coordinate
- `Y uint`: Y coordinate

## Examples

```go
func manhattanDistance(pos1, pos2 game.Position) int {
	dx := int(pos1.X) - int(pos2.X)
	dy := int(pos1.Y) - int(pos2.Y)
	if dx < 0 {
		dx = -dx
	}
	if dy < 0 {
		dy = -dy
	}
	return dx + dy
}
```

> [!TIP]
> The `Game` struct has a built-in `Distance()` method: `g.Distance(pos1, pos2)`.

## Related

- [Manhattan Distance Wikipedia](https://en.wikipedia.org/wiki/Taxicab_geometry)
