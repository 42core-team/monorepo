---
title: "⚙️ method Game.PrintObject(...)"
permalink: "go_PrintObject"
sidebarTitle: "⚙️ PrintObject()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/helpers.go

## Description

Prints all information about the current game state of a given object.

## Signature

```go
func (g *Game) PrintObject(obj *Object)
```

## Parameters

- `obj *Object`: The object to print information about

## Return

void

### stdout

```
---
ID: 188
Type: Unit
Position: (1, 2)
HP: 23
Unit Type: 0
Team ID: 43
Gems: 0
Action Cooldown: 3
```

## Examples

```go
units := g.TeamUnits()
for _, obj := range units {
	fmt.Println("Moving the next object!")
	g.PrintObject(obj)
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
