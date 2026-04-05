---
title: "⚙️ method Game.PrintObjects(...)"
permalink: "go_PrintObjects"
sidebarTitle: "⚙️ PrintObjects()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/helpers.go

## Description

Prints multiple objects.

## Signature

```go
func (g *Game) PrintObjects(objs []*Object)
```

## Parameters

- `objs []*Object`: The objects to print information about.

## Return

void

### stdout

```
---
ID: 160
Type: Unit
Position: (8, 7)
HP: 5
Unit Type: 0
Team ID: 43
Gems: 0
Action Cooldown: 0
---
---
ID: 168
Type: Unit
Position: (1, 2)
HP: 35
Unit Type: 0
Team ID: 43
Gems: 0
Action Cooldown: 4
---
...
```

## Examples

```go
deposits := g.ObjectsFilter(func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
g.PrintObjects(deposits)
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
