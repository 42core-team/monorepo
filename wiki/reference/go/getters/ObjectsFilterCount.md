---
title: "⚙️ method Game.ObjectsFilterCount(...)"
permalink: "go_ObjectsFilterCount"
sidebarTitle: "⚙️ ObjectsFilterCount()"
---

```go
func (g *Game) ObjectsFilterCount(predicate func(*game.Object) bool) uint
```

Counts objects matching a predicate without allocating a result slice. A nil predicate counts every object.
