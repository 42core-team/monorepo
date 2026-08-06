---
title: "⚙️ method Game.PrintObject(...)"
permalink: "go_PrintObject"
sidebarTitle: "⚙️ PrintObject()"
---

```go
func (g *Game) PrintObject(obj *game.Object)
func (g *Game) PrintObjects(objects []*game.Object)
```

Prints common and type-specific current state for debugging. `PrintObject(nil)` reports that the object is nil.
