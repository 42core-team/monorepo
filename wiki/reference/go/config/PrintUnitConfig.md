---
title: "⚙️ method Game.PrintUnitConfig(...)"
permalink: "go_PrintUnitConfig"
sidebarTitle: "⚙️ PrintUnitConfig()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/helpers.go

## Description

Prints a selected unit config.

[More Info on configs](documentation/configs)

## Signature

```go
func (g *Game) PrintUnitConfig(unitType UnitType)
```

## Parameters

- `unitType UnitType`: The type of unit to print the config for

## Return

void

### stdout

This code block may be an old config with values that do not apply to your current event - if you execute the function they will be correct.

```
Unit Warrior (0) Config:
  Cost: 150
  HP: 35
  Damage Core: 12
  Damage Unit: 6
  Damage Deposit: 4
  Damage Wall: 5
  Damage Bomb: 5
  Build Type: None
  Base Action Cooldown: 5
  Max Action Cooldown: 12
  Balance Per Cooldown Step: 12
```

## Examples

```go
fmt.Println("Warrior Config:")
g.PrintUnitConfig(game.UnitWarrior)
fmt.Println("Miner Config:")
g.PrintUnitConfig(game.UnitMiner)
fmt.Println("Carrier Config:")
g.PrintUnitConfig(game.UnitCarrier)
```

## Related

- [🔢 type UnitType](reference/go/objects/UnitType)
- [Configs Documentation](documentation/configs)
