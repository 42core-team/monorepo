---
title: "🔢 type ObjectType"
permalink: "go_ObjectType"
sidebarTitle: "🔢 ObjectType"
---

```go
const (
	ObjectCore ObjectType = iota
	ObjectUnit
	ObjectDeposit
	ObjectWall
	ObjectGemPile
)
```

The value determines which typed data accessor applies. Bomb and build object types belonged to an older ruleset and are no longer part of the Go client protocol.
