---
title: "⚙️ method Game.ObjectAtPos(...)"
permalink: "go_ObjectAtPos"
sidebarTitle: "⚙️ ObjectAtPos()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/game.go

## Description

Get any object based on its position.

## Signature

```go
func (g *Game) ObjectAtPos(pos Position) *Object
```

## Parameters

- `pos Position`: The position of the object the function should get

## Return

- `*Object`: The object at the position or nil if no such object exists.

## Examples

```go
func isPosEmpty(g *game.Game, pos game.Position) bool {
	return g.ObjectAtPos(pos) == nil
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct Position](reference/go/objects/Position)
