---
title: "⚙️ method Game.ObjectByID(...)"
permalink: "go_ObjectByID"
sidebarTitle: "⚙️ ObjectByID()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/game.go

## Description

Get any object based on its id.

## Signature

```go
func (g *Game) ObjectByID(id uint) (*Object, error)
```

## Parameters

- `id uint`: The id of the object the function should get

## Return

- `*Object`: The selected object with a matching id.
- `error`: An error if no object with the given id was found.

## Examples

```go
// we only need one carrier for now
var carrierID uint

bot.Run(func(g *game.Game, b *coregame.Bot) {
	if carrierID > 0 {
		carrier, err := g.ObjectByID(carrierID)
		if err == nil {
			// use the carrier
			_ = carrier
		}
	}
})
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
