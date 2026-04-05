---
title: "🔢 type ObjectType"
permalink: "go_ObjectType"
sidebarTitle: "🔢 ObjectType"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/types.go

## Description

Type of object

## Signature

```go
type ObjectType int

const (
	ObjectCore    ObjectType = iota // 0
	ObjectUnit                     // 1
	ObjectDeposit                  // 2
	ObjectWall                     // 3
	ObjectGemPile                  // 4
	ObjectBomb                     // 5
)
```

## Parameters

- **ObjectCore**: The namesake of CORE GAME. Your team's central hub. When yours is destroyed, you lose, when you destroy your opponents, you win. Also used to spawn new units if it has enough gems stored. There will only ever be one core per team.
- **ObjectUnit**: Your pawns, used to execute all the things you want to do in the game. Except for `Bot.CreateUnit`, all actions in the game are executed by units.
- **ObjectDeposit**: Gems encased in stone. Use a miner or another unit to mine it, making the gems drop as a gem pile. Will generally have significantly more gems stored than gem piles spawning normally.
- **ObjectWall**: It's there to be in your way. Walk around it or mine through it.
- **ObjectGemPile**: Gems lying around on the floor.
- **ObjectBomb**: Careful!

## Examples

```go
isDeposit := func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
}
isGems := func(obj *game.Object) bool {
	return obj.Type == game.ObjectGemPile
}
isDepositOrGems := func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit || obj.Type == game.ObjectGemPile
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
