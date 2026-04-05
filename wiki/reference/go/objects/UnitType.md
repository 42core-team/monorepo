---
title: "🔢 type UnitType"
permalink: "go_UnitType"
sidebarTitle: "🔢 UnitType"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/types.go

## Description

Type of unit. Which are available differs from event to event.

There is an overview of all the unit types and their properties and settings in the config.

> [!TIP]
> Type `game.Unit` and autocomplete will show you all the available units.

## Signature

```go
type UnitType int

const (
	UnitWarrior UnitType = iota // 0
	UnitMiner                  // 1
	UnitCarrier                // 2
	// ... (it depends)
)
```

## Parameters

To find out what each unit is good or bad at, check out the config.

Basic units that are in almost every config / event are:

- **UnitWarrior**: Offensive melee unit, good at attacking other units and cores.
- **UnitMiner**: Pickaxe-wielding worker, good at destroying gem deposits & walls.
- **UnitCarrier**: Light-footed transporter, fast & does not get slowed down when carrying lots of gems as quickly as other units.

Note this info may be inaccurate, please check your config.

## Examples

```go
data := obj.GetUnitData()
if data != nil {
	switch data.UnitType {
	case game.UnitWarrior:
		// ...
	case game.UnitMiner:
		// ...
	}
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct UnitConfig](reference/go/config/UnitConfig)
