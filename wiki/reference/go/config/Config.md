---
title: "🧩 struct Config"
permalink: "go_Config"
sidebarTitle: "🧩 Config"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/config.go

## Description

Immutable info on how certain aspects of the game are set up.

[More Info on configs](documentation/configs)

## Signature

```go
type Config struct {
	GridSize          uint
	IdleIncome        uint
	IdleIncomeTimeout uint
	DepositHp         uint
	DepositIncome     uint
	GemPileIncome     uint
	CoreHp            uint
	CoreSpawnCooldown uint
	InitialBalance    uint
	WallHp            uint
	WallBuildCost     uint
	BombCountdown     uint
	BombThrowCost     uint
	BombReach         uint
	BombDamageCore    uint
	BombDamageUnit    uint
	BombDamageDeposit uint
	Units             []*UnitConfig
}
```

## Parameters

- `GridSize uint`: The width & height of the map. It's always square.
- `IdleIncome uint`: How much idle income you get every second.
- `IdleIncomeTimeout uint`: After how many ticks from the start of the game you'll stop receiving idle income.
- `DepositHp uint`: How much healthpoints a deposit has at the start of the game.
- `DepositIncome uint`: A gem pile object with what balance the deposit will drop at its previous position when it is destroyed.
- `GemPileIncome uint`: How many gems a default gem pile object contains.
- `CoreHp uint`: How much healthpoints a core has at the start of the game.
- `CoreSpawnCooldown uint`: Ticks it takes after a unit was spawned before core can spawn another unit.
- `InitialBalance uint`: How many gems in their core a team starts with.
- `WallHp uint`: How many healthpoints a wall has at the start of the game.
- `WallBuildCost uint`: How much it costs for a builder to build a wall.
- `BombCountdown uint`: How many ticks a bomb takes to explode after being thrown.
- `BombThrowCost uint`: How much it costs to throw a bomb.
- `BombReach uint`: How big the explosion of a bomb is. See the [bombs documentation](documentation/bombs) page for more info on the bomb explosion pattern.
- `BombDamageCore uint`: How much damage a bomb does to cores hit by its explosion.
- `BombDamageUnit uint`: How much damage a bomb does to units hit by its explosion.
- `BombDamageDeposit uint`: How much damage a bomb does to deposits hit by its explosion.
- `Units []*UnitConfig`: Slice of all unit types that are available in the game.

## Examples

```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	if g.ElapsedTicks < g.Config.IdleIncomeTimeout {
		// focus less on miner & carrier spawning
		// ...
	}
})
```

## Related

- [🧩 struct UnitConfig](reference/go/config/UnitConfig)
- [Configs Documentation](documentation/configs)
