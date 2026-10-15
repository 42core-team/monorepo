---
title: "My First CORE Bot in Go"
permalink: "my_first_core_bot_go"
sidebarTitle: "My First Go Bot"
---

This guide builds a Go bot that creates a component-based unit and sends it toward the opponent core.

## Create a unit design

CORE no longer has fixed warrior, miner, or carrier enums. Use your event's Unit Builder to choose valid component IDs. Component availability, costs, limits, and combinations can differ between events.

```go
if err := core.CreateUnit("Warrior", "combat", "siege", "health"); err != nil {
	log.Printf("create unit: %v", err)
}
```

The first argument is the custom unit name. Pass an empty string to receive a generated name. The remaining arguments are component IDs. Repeated components are allowed only if the Unit Builder accepts the design.

Creation is queued for execution between ticks. It can fail when your core is on spawn cooldown, lacks gems, has no spawn position, or receives an invalid component design. See [`CreateUnit`](reference/go/actions/CreateUnit).

Action functions return errors for invalid local inputs or an unavailable client. A nil error does not guarantee execution: gameplay checks remain on the server. Server error messages are printed by the library. `StartGame` returns transport and protocol failures rather than treating a disconnect as successful completion.

## Run the game loop

```go
func tick(elapsedTicks uint) {
	// Read core.Game and queue actions with core.CreateUnit, core.Travel, etc.
}

func main() {
	cfg := core.DefaultBotConfig(teamID, "My CORE Bot")
	if err := core.StartGame(cfg, tick); err != nil {
		log.Fatal(err)
	}
}
```

Import `"github.com/42core-team/go-client-lib"` to use the package named `core`.
Like C, it has no public bot instance: `core.StartGame` manages the connection,
`core.Game` exposes current state, and actions/getters are package-level functions.
Only one game can run per process. Read state and call actions/getters/debug
helpers from your callback, not concurrent goroutines. Treat state as read-only.

`StartGame` calls your callback after every state update and closes the connection
on return. Objects created now appear on a later tick. Keep object IDs, rather
than object pointers, when state must persist between ticks.

## Travel toward a target

```go
if err := core.Travel(unit, target.Pos, nil); err != nil {
	log.Printf("travel: %v", err)
}
```

One call queues at most one adjacent move or attack. A nil policy uses the component-aware default: open positions cost one, friendly cores are blocked, friendly units are passed after waiting, and breakable obstacles cost their estimated attack count plus one. The policy uses `DamageUnit`, `DamageCore`, `DamageObject`, damage reduction, and `MaxGems` from the unit state. Travel also adds the selected route to the unit's visualizer debug path.

Travel intentionally leaves action-cooldown checks to your code. Call it only for ready units:

```go
data, err := unit.UnitData()
if err != nil {
	log.Printf("read unit: %v", err)
	return
}
if data.ActionCooldown <= 0 {
	if err := core.Travel(unit, target.Pos, nil); err != nil {
		log.Printf("travel: %v", err)
	}
}
```

A custom policy receives each grid position and the traveling unit:

```go
err := core.Travel(unit, target.Pos, func(pos game.Position, unit *game.Object) core.TileTravelability {
	if dangerous(pos) {
		return core.TileTravelability{Action: core.TravelBlock}
	}
	return core.TileTravelability{Weight: 1, Action: core.TravelPass}
})
if err != nil {
	log.Printf("travel: %v", err)
}
```

`TravelBlock` excludes a tile, `TravelPass` routes through it without attacking its occupant, and `TravelAttack` attacks its occupant when it becomes the next step. See [`Travel`](reference/go/actions/Travel).

## Complete example

Replace the template's `my-core-bot/main.go` with this example and keep `getters.go` alongside it. `ownCore`, `opponentCore`, and `ownUnits` are editable template helpers, not library methods.

```go
package main

import (
	"fmt"
	"log"
	"os"
	"strconv"

	"github.com/42core-team/go-client-lib"
)

func tick(elapsedTicks uint) {
	if own := ownCore(); own != nil {
		data, err := own.CoreData()
		if err != nil {
			log.Printf("read core: %v", err)
		} else if data.SpawnCooldown == 0 {
			if err := core.CreateUnit("Warrior", "combat"); err != nil {
				log.Printf("create unit: %v", err)
			}
		}
	}
	target := opponentCore()
	for _, unit := range ownUnits() {
		data, err := unit.UnitData()
		if err != nil {
			log.Printf("read unit: %v", err)
			continue
		}
		if target != nil && data.ActionCooldown <= 0 {
			if err := core.Travel(unit, target.Pos, nil); err != nil {
				log.Printf("travel: %v", err)
			}
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
	if err := core.StartGame(core.DefaultBotConfig(teamID, "My CORE Bot"), tick); err != nil {
		fmt.Println(err)
	}
}
```

Replace `combat` with a valid component list from your event. Continue with the [Unit Builder guide](documentation/unit_builder), [getter filtering](documentation/getter_filtering), and the [Go API reference](reference/go/Game).
