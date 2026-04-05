---
title: "⚙️ method Bot.AddObjectInfo(...)"
permalink: "go_AddObjectInfo"
sidebarTitle: "⚙️ AddObjectInfo()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/debug.go

## Description

Attaches an arbitrary string to any object (typically a unit or your core) that will be displayed in the object's tooltip if someone hovers over the object in the visualizer. May be helpful for a variety of debugging purposes.

This function can be called multiple times in a given tick, all of the strings the function is called with will be appended together at the end.

> [!WARNING]
> Opponents cannot see your debug data during the game, but may be able to view it once the game has concluded through the visualizer. (Better not reveal your super-secret strategy...)

## Signature

```go
func (b *Bot) AddObjectInfo(obj *game.Object, info string)
```

## Parameters

- `obj *game.Object`: The object which the debug info will be attached to.
- `info string`: The string which will be attached to the debug data.

## Return

void (nil-safe: does nothing if obj is nil)

## Examples

```go
enemyCore := g.EnemyCore()
if enemyCore != nil {
	b.AddObjectInfo(unit, fmt.Sprintf("I am a warrior! - Heading for enemy core at [%d,%d]!\n", enemyCore.Pos.X, enemyCore.Pos.Y))
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
