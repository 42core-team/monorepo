---
title: "⚙️ method Game.NearestObject(...)"
permalink: "go_NearestObject"
sidebarTitle: "⚙️ NearestObject()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/game.go

## Description

Get the nearest object to a given position matching a custom filtering predicate.

[Manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry) is used to determine distance. If two objects or more have the same distance, either could be picked.

## Signature

```go
func (g *Game) NearestObject(fromPos Position, predicate func(*Object) bool) *Object
```

## Parameters

- `fromPos Position`: Position to search from
- `predicate func(*Object) bool`: Function returning true if the object should be considered

## Return

- `*Object`: The nearest object that matches the predicate or nil if no such object exists.

## Examples

```go
isDeposit := func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
}
nearestDeposit := g.NearestObject(miner.Pos, isDeposit)
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct Position](reference/go/objects/Position)
