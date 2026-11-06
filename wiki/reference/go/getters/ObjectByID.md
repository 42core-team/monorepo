---
title: "⚙️ function ObjectByID(...)"
permalink: "go_ObjectByID"
sidebarTitle: "⚙️ ObjectByID()"
---

```go
func ObjectByID(id uint) (*game.Object, error)
```

Returns the current object with `id`, or an error when it is absent.

```go
object, err := core.ObjectByID(savedID)
if err != nil { return }
```
