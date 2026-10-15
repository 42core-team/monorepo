---
title: "🧩 type Position"
permalink: "go_Position"
sidebarTitle: "🧩 Position"
---

```go
type Position struct { X, Y uint }
func NewPosition(x, y uint) Position
func IsPosValid(pos Position) bool
func Distance(a, b Position) int
```

Coordinates are zero indexed and valid from zero through `core.Game.Config.GridSize-1`. `Distance` returns Manhattan distance.
