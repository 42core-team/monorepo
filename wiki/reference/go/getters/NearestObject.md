---
title: "⚙️ function NearestObject(...)"
permalink: "go_NearestObject"
sidebarTitle: "⚙️ NearestObject()"
---

```go
func NearestObject(from game.Position, predicate func(*game.Object) bool) *game.Object
```

Returns the matching object with the smallest Manhattan distance, or nil when none matches. A nil predicate returns nil.
