---
title: "⚙️ method Bot.AddObjectPathStep(...)"
permalink: "go_AddObjectPathStep"
sidebarTitle: "⚙️ AddObjectPathStep()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/debug.go

## Description

Used to build a debug path which displays where a unit will move next in the visualizer. This is done by calling the function multiple times, one for each grid tile the unit is planning on traversing, which will then be appended together by the core library for your convenience.

> [!WARNING]
> Opponents cannot see your debug path during the game, but may be able to view it once the game has concluded through the visualizer.

## Signature

```go
func (b *Bot) AddObjectPathStep(unit *game.Object, pos game.Position)
```

## Parameters

- `unit *game.Object`: The unit which the debug path step will be attached to.
- `pos game.Position`: The position the unit is planning to move to.

## Return

void (nil-safe: does nothing if unit is nil)

## Examples

```go
for _, step := range plannedPath {
	b.AddObjectPathStep(unit, step)
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct Position](reference/go/objects/Position)
