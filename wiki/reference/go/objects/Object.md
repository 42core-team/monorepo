---
title: "🧩 type Object"
permalink: "go_Object"
sidebarTitle: "🧩 Object"
---

```go
type Object struct {
	ID         uint
	Type       ObjectType
	Pos        Position
	Hp         uint
	TeamID     uint
	ObjectData ObjectData
	Data       any
}
```

`Data` belongs to bot code and persists while the object remains in the game. Use the typed accessors for server-owned details:

```go
type UnitData struct {
	TeamID         uint
	Gems           uint
	ActionCooldown int
	Components     []string
	Properties     UnitProperties
	Name           string
}

type CoreData struct {
	TeamID uint; Gems uint; SpawnCooldown uint
}

type DepositData struct { Gems uint }
```

`ActionCooldown` is signed: zero and negative values mean the unit is ready. Accessors are `GetUnitData`, `GetCoreData`, and `GetDepositData`. Helpers include `IsAlive`, `IsEnemy`, `IsFriendly`, and `IsOfType`.
