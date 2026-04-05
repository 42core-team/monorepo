---
title: "🧩 struct UnitConfig"
permalink: "go_UnitConfig"
sidebarTitle: "🧩 UnitConfig"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/config.go

## Description

Immutable info on what each type of unit is able to do.

[More Info on configs](documentation/configs)

## Signature

```go
type UnitConfig struct {
	Name               string
	UnitType           UnitType
	Cost               uint
	Hp                 uint
	BaseActionCooldown uint
	MaxActionCooldown  uint
	BalancePerCooldown uint
	DamageCore         uint
	DamageUnit         uint
	DamageDeposit      uint
	DamageWall         uint
	DamageBomb         uint
	BuildType          BuildType
}
```

## Parameters

- `Name string`: The name of the unit.
- `UnitType UnitType`: The unit type of the unit.
- `Cost uint`: What the unit costs to create.
- `Hp uint`: How much healthpoints the unit has.
- `BaseActionCooldown uint`: The time a unit waits between moves if it is not carrying any gems.
- `MaxActionCooldown uint`: The maximum boundary of a units wait time between moves if it's carrying a ton of gems.
- `BalancePerCooldown uint`: Defines increase of delay between action executions.
- `DamageCore uint`: How much damage the unit deals to cores.
- `DamageUnit uint`: How much damage the unit deals to units.
- `DamageDeposit uint`: How much damage the unit deals to deposits.
- `DamageWall uint`: How much damage the unit deals to walls.
- `DamageBomb uint`: How much damage the unit deals to bombs.
- `BuildType BuildType`: The units build type. If `BuildTypeNone`, the unit can't build.

## Examples

```go
uconf := g.Config.GetUnitConfig(game.UnitMiner)
myCore := g.MyCore()
if myCore != nil && uconf != nil {
	coreData := myCore.GetCoreData()
	if coreData != nil && coreData.Gems >= uconf.Cost {
		b.CreateUnit(game.UnitMiner)
	}
}
```

## Related

- [🔢 type BuildType](reference/go/objects/BuildType)
- [Configs Documentation](documentation/configs)
