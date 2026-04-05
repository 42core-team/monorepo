---
title: "⚙️ method Bot.SimplePathfind(...)"
permalink: "go_SimplePathfind"
sidebarTitle: "⚙️ SimplePathfind()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/actions.go

## Description

Very simple pathfinding logic implementation. It won't help you win, but it can be used to easily get started while you figure out how the game works.

When you call it, it will determine the next move to make. It automatically attacks enemy objects in its way (provided they aren't your units or core) and returns the position to move to. **You need to call `Bot.Move` with the returned position yourself.**

It is encouraged that you write your own pathfinder eventually for a good bot using [`Bot.Move`](reference/go/actions/Move) and [`Bot.Attack`](reference/go/actions/Attack), this function is just a util for you to easily get started.

## Signature

```go
func (b *Bot) SimplePathfind(unit *game.Object, targetPos game.Position) game.Position
```

## Parameters

- `unit *game.Object`: The unit that should move
- `targetPos game.Position`: The position the unit should pathfind towards

## Return

- `game.Position`: The next position the unit should move to. Call `Bot.Move` with this position. Returns the unit's current position if no move is possible.

## Examples

```go
teamWarriors := g.ObjectsFilter(func(obj *game.Object) bool {
	data := obj.GetUnitData()
	return obj.Type == game.ObjectUnit && data != nil && data.TeamID == g.MyTeamID && data.UnitType == game.UnitWarrior
})
for _, warrior := range teamWarriors {
	enemyCore := g.EnemyCore()
	if enemyCore != nil {
		nextPos := b.SimplePathfind(warrior, enemyCore.Pos)
		b.Move(warrior, nextPos)
	}
}
```

## Related

- [🧩 struct Object](reference/go/objects/Object)
- [🧩 struct Position](reference/go/objects/Position)
