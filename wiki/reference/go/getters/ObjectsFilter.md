---
title: "⚙️ function ObjectsFilter(...)"
permalink: "go_ObjectsFilter"
sidebarTitle: "⚙️ ObjectsFilter()"
---

```go
func ObjectsFilter(predicate func(*game.Object) bool) []*game.Object
```

Returns every object for which the predicate is true. A nil predicate returns nil.

```go
walls := core.ObjectsFilter(func(obj *game.Object) bool { return obj.Type == game.ObjectWall })
```
