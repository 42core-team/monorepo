---
title: "⚙️ method Game.ObjectAtPos(...)"
permalink: "go_ObjectAtPos"
sidebarTitle: "⚙️ ObjectAtPos()"
---

```go
func (g *Game) ObjectAtPos(pos game.Position) *game.Object
```

Returns the object at a position, or nil when it is empty.

```go
if obstacle := g.ObjectAtPos(next); obstacle != nil { bot.Attack(unit, obstacle) }
```
