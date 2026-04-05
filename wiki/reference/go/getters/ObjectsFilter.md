---
title: "⚙️ method Game.ObjectsFilter(...)"
permalink: "go_ObjectsFilter"
sidebarTitle: "⚙️ ObjectsFilter()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/game.go

## Description

Get all objects matching a custom filtering predicate.

Unlike the C version, Go manages memory automatically - no need to free the returned slice.

To count matching objects, use `len(g.ObjectsFilter(...))`.

## Signature

```go
func (g *Game) ObjectsFilter(predicate func(*Object) bool) []*Object
```

## Parameters

- `predicate func(*Object) bool`: Function returning true if the object should be included in the result

## Return

- `[]*Object`: Slice of selected objects. Empty slice if no objects match. No need to free.

## Examples

```go
isWarrior := func(obj *game.Object) bool {
	data := obj.GetUnitData()
	return obj.Type == game.ObjectUnit && data != nil && data.UnitType == game.UnitWarrior
}
warriors := g.ObjectsFilter(isWarrior)
for _, warrior := range warriors {
	// warrior move & attack logic
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
