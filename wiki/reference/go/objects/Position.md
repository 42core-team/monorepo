---
title: "🧩 type Position"
permalink: "go_Position"
sidebarTitle: "🧩 Position"
---

```go
type Position struct { X, Y uint }
func NewPosition(x, y uint) Position
func (g *Game) IsPosValid(pos Position) bool
func (g *Game) Distance(a, b Position) int
```

Coordinates are zero indexed and valid from zero through `g.Config.GridSize-1`. `Distance` returns Manhattan distance.
