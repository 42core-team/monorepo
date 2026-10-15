---
title: "⚙️ Go debug methods"
permalink: "go_Debug"
sidebarTitle: "⚙️ Debug methods"
---

```go
func AddObjectInfo(obj *game.Object, format string, args ...any)
func AddObjectPathStep(unit *game.Object, pos game.Position)
```

Debug data is sent with the next action packet and then reset. Repeated info calls append formatted text; repeated path calls append positions.

```go
core.AddObjectInfo(unit, "target: %d at (%d,%d)\n", target.ID, target.Pos.X, target.Pos.Y)
core.AddObjectPathStep(unit, target.Pos)
```

Nil objects are ignored.
