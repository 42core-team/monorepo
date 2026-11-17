---
title: "⚙️ Go debug methods"
permalink: "go_Debug"
sidebarTitle: "⚙️ Debug methods"
---

```go
func AddObjectInfo(obj *game.Object, format string, args ...any)
func AddObjectPathStep(unit *game.Object, pos game.Position)
```

Debug data is sent with the next action packet and then reset. Repeated info calls append formatted text; repeated path calls append positions. The server keeps at most 1024 bytes of debug info and 100 debug path points per team each tick and truncates the rest; you get a "Debug Error" message when this happens.

```go
core.AddObjectInfo(unit, "target: %d at (%d,%d)\n", target.ID, target.Pos.X, target.Pos.Y)
core.AddObjectPathStep(unit, target.Pos)
```

Nil objects are ignored.
