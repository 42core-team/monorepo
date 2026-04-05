---
title: "⚙️ method Config.GetUnitConfig(...)"
permalink: "go_GetUnitConfig"
sidebarTitle: "⚙️ GetUnitConfig()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/config.go

## Description

Get the unit config for a specific unit type.

[More Info on configs](documentation/configs)

## Signature

```go
func (c *Config) GetUnitConfig(unitType UnitType) *UnitConfig
```

## Parameters

- `unitType UnitType`: The type of unit to get the config for

## Return

- `*UnitConfig`: The unit config or nil if no such unit type exists.

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

- [🧩 struct UnitConfig](reference/go/config/UnitConfig)
- [🔢 type UnitType](reference/go/objects/UnitType)
- [Configs Documentation](documentation/configs)
