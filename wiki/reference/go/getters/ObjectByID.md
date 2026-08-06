---
title: "⚙️ method Game.ObjectByID(...)"
permalink: "go_ObjectByID"
sidebarTitle: "⚙️ ObjectByID()"
---

```go
func (g *Game) ObjectByID(id uint) (*game.Object, error)
```

Returns the current object with `id`, or an error when it is absent.

```go
object, err := g.ObjectByID(savedID)
if err != nil { return }
```
