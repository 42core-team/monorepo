---
title: "⚙️ method Bot.Build(...)"
permalink: "go_Build"
sidebarTitle: "⚙️ Build()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Builds a new object.

The `BuildType` of the builder unit in the config will determine what gets built (e.g. bomb or wall). ([More info on bombs](documentation/bombs)).

The builder must hold enough money to be able to afford the object it is building as defined by the [config](documentation/configs).

- Objects can only be built one tile up, down, left or right from the builder; for more see [Action Position Limits](documentation/action_position_limits).
- Builders objects can only build if their action cooldown is 0 or less, for more see [Cooldowns](documentation/cooldowns).

## Signature

```go
func (b *Bot) Build(builder *game.Object, pos game.Position)
```

## Parameters

- `builder *game.Object`: The unit object that has a unit type that is able to build. Must have a `BuildType` that is not `BuildTypeNone` in the config.
- `pos game.Position`: The position to build the object at.

## Return

void (nil-safe: does nothing if builder is nil)

## Examples

```go
unit := getWallBuilderUnit(g)
udata := unit.GetUnitData()
if unit != nil && udata != nil && *udata.Gems >= g.Config.WallBuildCost {
	b.Build(unit, game.Position{X: unit.Pos.X + 1, Y: unit.Pos.Y})
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🔢 type BuildType](reference/go/objects/BuildType)
- [🧩 struct Position](reference/go/objects/Position)
