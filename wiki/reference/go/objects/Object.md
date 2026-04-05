---
title: "🧩 struct Object"
permalink: "go_Object"
sidebarTitle: "🧩 Object"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/types.go

## Description

Game object structure representing all the things that can be present on the game field.

- There are never two objects with the same id.
- There are never more than one object at a given [grid position](reference/go/objects/Position).

> [!TIP]
> The `Data any` field is **especially powerful & useful**. The library will never touch this field, it's yours to mess with for whatever you want.
> It's generally used to store specific jobs, tasks or targets of the unit in the easiest way possible.

To access type-specific data, use the accessor methods: `GetUnitData()`, `GetCoreData()`, `GetDepositData()`, `GetBombData()`.

## Signature

```go
type Object struct {
	ID         uint
	Type       ObjectType
	Pos        Position
	Hp         int32
	TeamID     uint
	ObjectData ObjectData // use accessor methods below
	Data       any        // custom data, save whatever you want here
}

// Accessor methods
func (o *Object) GetUnitData() *UnitData
func (o *Object) GetCoreData() *CoreData
func (o *Object) GetDepositData() *DepositData
func (o *Object) GetBombData() *BombData

// Helper methods
func (o *Object) IsAlive() bool
func (o *Object) IsEnemy(myTeamID uint) bool
func (o *Object) IsFriendly(myTeamID uint) bool
func (o *Object) IsOfType(objType ObjectType) bool
```

## Type-Specific Data

### UnitData
```go
type UnitData struct {
	UnitType       UnitType
	TeamID         uint
	Gems           *uint    // pointer - may be nil
	ActionCooldown *uint    // pointer - may be nil
}
```

### CoreData
```go
type CoreData struct {
	TeamID        uint
	Gems          uint
	SpawnCooldown uint
}
```

### DepositData
```go
type DepositData struct {
	Gems uint
}
```

### BombData
```go
type BombData struct {
	Countdown uint
}
```

## Parameters

- `ID uint`: The unique id of the object
- `Type ObjectType`: Type of the object
- `Pos Position`: The position of the object
- `Hp int32`: The current healthpoints of the object
- `TeamID uint`: The team ID (for units and cores)
- `Data any`: Custom data, _save whatever you want here_, it will persist across ticks.

## Examples

```go
// move units
units := g.TeamUnits()
for _, obj := range units {
	data := obj.GetUnitData()
	if data == nil {
		continue
	}

	switch data.UnitType {
	case game.UnitWarrior:
		// ...
	case game.UnitMiner:
		// ...
	}
}
```

## Related

- [🔢 type ObjectType](reference/go/objects/ObjectType)
- [🧩 struct Position](reference/go/objects/Position)
- [🔢 type UnitType](reference/go/objects/UnitType)
