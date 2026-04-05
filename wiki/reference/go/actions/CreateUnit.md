---
title: "⚙️ method Bot.CreateUnit(...)"
permalink: "go_CreateUnit"
sidebarTitle: "⚙️ CreateUnit()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Creates a new unit of specified type.

Your core needs to own a certain amount of money to be able to create a unit. This amount is different depending on the [type of unit](reference/go/objects/UnitType) and can be found in the [config](documentation/configs).

Cores with a spawn cooldown that isn't 0 are unable to spawn new units. [More info on spawn cooldowns](documentation/cooldowns).

Units are spawned as close to your core as possible - but if all positions directly next to your core are occupied, a flood fill algorithm will be used looking for the next empty space and the unit will be placed there.

## Signature

```go
func (b *Bot) CreateUnit(unitType game.UnitType)
```

## Parameters

- `unitType game.UnitType`: The type of unit to create

## Return

void

## Examples

```go
warriors := g.ObjectsFilter(func(obj *game.Object) bool {
	data := obj.GetUnitData()
	return obj.Type == game.ObjectUnit && data != nil && data.TeamID == g.MyTeamID && data.UnitType == game.UnitWarrior
})
if len(warriors) < 3 {
	b.CreateUnit(game.UnitWarrior)
}
```

## Related

- [🔢 type UnitType](reference/go/objects/UnitType)
