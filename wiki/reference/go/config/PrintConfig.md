---
title: "⚙️ method Game.PrintConfig(...)"
permalink: "go_PrintConfig"
sidebarTitle: "⚙️ PrintConfig()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/game/helpers.go

## Description

Prints the entire game config and all unit configs.

This is the Go equivalent of both `core_print_config` and `core_print_config_game` in C.

[More Info on configs](documentation/configs)

## Signature

```go
func (g *Game) PrintConfig()
```

## Parameters

void

## Return

void

### stdout

This code block may be an old config with values that do not apply to your current event - if you execute the function they will be correct.

```
Game Config:
  Grid Size: 20
  Idle Income: 2
  Idle Income Timeout: 1000
  Deposit HP: 50
  Deposit Income: 150
  Core HP: 200
  Initial Balance: 200
  Wall HP: 50
  Wall Build Cost: 20
  Bomb Countdown: 10
  Bomb Throw Cost: 50
  Bomb Reach: 3
  Bomb Damage Core: 50
  Bomb Damage Unit: 30
  Bomb Damage Deposit: 40
Unit Warrior (0) Config:
	...
Unit Miner (1) Config:
	...
Unit Carrier (2) Config:
	...
```

## Examples

```go
bot.Run(func(g *game.Game, b *coregame.Bot) {
	if g.ElapsedTicks == 0 {
		g.PrintConfig()
	}
	// ...
})
```

## Related

- [Configs Documentation](documentation/configs)
