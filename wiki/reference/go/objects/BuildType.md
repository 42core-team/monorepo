---
title: "🔢 type BuildType"
permalink: "go_BuildType"
sidebarTitle: "🔢 BuildType"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/config.go

## Description

Determines whether and what the unit will build if you use it as the builder argument when executing a build action.

This type is only used in the config to indicate each units building capabilities.

## Signature

```go
type BuildType int

const (
	BuildTypeNone BuildType = iota // 0
	BuildTypeWall                  // 1
	BuildTypeBomb                  // 2
)
```

## Parameters

- **BuildTypeNone**: The default unit build setting, if you call `Bot.Build` with this, it simply won't do anything.
- **BuildTypeWall**: This unit is able to build walls.
- **BuildTypeBomb**: This unit is able to create bombs.

## Examples

```go
uconf := g.Config.GetUnitConfig(data.UnitType)
if uconf != nil && uconf.BuildType == game.BuildTypeNone {
	continue
}
```

## Related

- [🧩 struct UnitConfig](reference/go/config/UnitConfig)
- [⚙️ method Bot.Build(...)](reference/go/actions/Build)
