---
title: "⚙️ function ObjectAtPos(...)"
permalink: "go_ObjectAtPos"
sidebarTitle: "⚙️ ObjectAtPos()"
---

```go
func ObjectAtPos(pos game.Position) *game.Object
```

Returns the object at a position, or nil when it is empty.

```go
if obstacle := core.ObjectAtPos(next); obstacle != nil { core.Attack(unit, obstacle) }
```
