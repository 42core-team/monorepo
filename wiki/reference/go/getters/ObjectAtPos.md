---
title: "⚙️ function ObjectAtPos(...)"
permalink: "go_ObjectAtPos"
sidebarTitle: "⚙️ ObjectAtPos()"
---

```go
func ObjectAtPos(pos game.Position) *game.Object
```

Returns the object at a position, or nil when it is empty or off the board.
Like `core_get_obj_from_pos` in C, it uses a position index, built once per
tick on the first lookup, so it is cheap to call for every tile, for example
from a custom `TravelPolicy`.

```go
if obstacle := core.ObjectAtPos(next); obstacle != nil { core.Attack(unit, obstacle) }
```
