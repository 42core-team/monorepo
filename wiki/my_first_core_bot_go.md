---
title: "My First CORE Bot in Go"
permalink: "my_first_core_bot_go"
sidebarTitle: "My First Go Bot"
---

This guide builds a Go bot that creates a component-based unit and sends it toward the opponent core.

## Create a unit design

CORE no longer has fixed warrior, miner, or carrier enums. Use your event's Unit Builder to choose valid component IDs. Component availability, costs, limits, and combinations can differ between events.

```go
bot.CreateUnit("Warrior", "combat", "siege", "health")
```

The first argument is the custom unit name. Pass an empty string to receive a generated name. The remaining arguments are component IDs. Repeated components are allowed only if the Unit Builder accepts the design.

Creation is queued for execution between ticks. It can fail when your core is on spawn cooldown, lacks gems, has no spawn position, or receives an invalid component design. See [`Bot.CreateUnit`](reference/go/actions/CreateUnit).

## Run the game loop

```go
func tick(g *game.Game, bot *coregame.Bot) {
	// Read g and queue actions with bot.
}

func main() {
	cfg := coregame.DefaultBotConfig(teamID, "My CORE Bot")
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		log.Fatal(err)
	}
	if err := bot.Run(tick); err != nil {
		log.Fatal(err)
	}
}
```

`Run` calls your callback after every state update. Objects created now appear on a later tick. Keep object IDs, rather than object pointers, when state must persist between ticks.

## Travel toward a target

```go
bot.Travel(unit, target.Pos, nil)
```

One call queues at most one adjacent move or attack. A nil policy uses the component-aware default: open positions cost one, friendly cores are blocked, friendly units are passed after waiting, and breakable obstacles cost their estimated attack count plus one. The policy uses `DamageUnit`, `DamageCore`, `DamageObject`, damage reduction, and `MaxGems` from the unit state. Travel also adds the selected route to the unit's visualizer debug path.

Travel intentionally leaves action-cooldown checks to your code. Call it only for ready units:

```go
if unit.GetUnitData().ActionCooldown <= 0 {
	bot.Travel(unit, target.Pos, nil)
}
```

A custom policy receives each grid position and the traveling unit:

```go
bot.Travel(unit, target.Pos, func(pos game.Position, unit *game.Object) coregame.TileTravelability {
	if dangerous(pos) {
		return coregame.TileTravelability{Action: coregame.TravelBlock}
	}
	return coregame.TileTravelability{Weight: 1, Action: coregame.TravelPass}
})
```

`TravelBlock` excludes a tile, `TravelPass` routes through it without attacking its occupant, and `TravelAttack` attacks its occupant when it becomes the next step. See [`Bot.Travel`](reference/go/actions/Travel).

## Complete example

```go
package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func tick(g *game.Game, bot *coregame.Bot) {
	if core := g.MyCore(); core != nil && core.GetCoreData().SpawnCooldown == 0 {
		bot.CreateUnit("Warrior", "combat")
	}
	target := g.EnemyCore()
	for _, unit := range g.TeamUnits() {
		if target != nil && unit.GetUnitData().ActionCooldown <= 0 {
			bot.Travel(unit, target.Pos, nil)
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("usage: bot <team-id>")
		os.Exit(1)
	}
	teamID, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
	bot, err := coregame.NewBot(coregame.DefaultBotConfig(teamID, "My CORE Bot"))
	if err == nil {
		err = bot.Run(tick)
	}
	if err != nil {
		fmt.Println(err)
	}
}
```

Replace `combat` with a valid component list from your event. Continue with the [Unit Builder guide](documentation/unit_builder), [getter filtering](documentation/getter_filtering), and the [Go API reference](reference/go/Game).
