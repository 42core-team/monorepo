---
title: "🧩 struct Game"
permalink: "go_Game"
sidebarTitle: "🧩 Game"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/game.go

## Description

The Game struct is your primary source of information about the happenings in the game. It contains the entire game state.

The Game struct is passed to your tick callback function as the first argument. Read it to your liking!

## Signature

```go
type Game struct {
	Config       Config
	Objects      []*Object
	ElapsedTicks uint
	MyTeamID     uint
}
```

## Parameters

- `Config Config`: The config contains base information about the game that don't change such as the map size and the unit types.
- `Objects []*Object`: Slice of all objects (units, cores, deposits, gem piles, bombs, walls etc.) and their information.
- `ElapsedTicks uint`: The elapsed ticks since the game started.
- `MyTeamID uint`: The id of the team that you are playing for.

## Examples

```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	for _, obj := range g.Objects {
		// do something with the object here
	}
})
```

## Related

- [🧩 struct Config](reference/go/config/Config)
- [🧩 struct Object](reference/go/objects/Object)
