---
title: "⚙️ method Bot.Travel(...)"
permalink: "go_Travel"
sidebarTitle: "⚙️ Travel()"
---

Queues one optimal move or attack toward a destination and adds the selected route to the unit's visualizer debug path.

```go
type TravelAction int

const (
	TravelBlock TravelAction = iota
	TravelPass
	TravelAttack
)

type TileTravelability struct {
	Weight uint
	Action TravelAction
}

type TravelPolicy func(pos game.Position, unit *game.Object) TileTravelability
func (b *Bot) Travel(unit *game.Object, target game.Position, policy TravelPolicy) error
```

Pass nil for the default component-aware policy. It blocks friendly cores, waits for friendly units, estimates attack costs from effective damage and HP, and blocks gem piles when the unit has reached `MaxGems`.

Returns errors for an unavailable client, invalid unit data, or out-of-bounds origin/destination. Arrival or no available route queues nothing and returns nil. Otherwise, nil means an action was queued or travel is waiting for an occupant; it does not guarantee server execution. Server error messages are printed by the library.

Travel does not check the traveling unit's action cooldown. Check it before calling:

```go
data, err := unit.UnitData()
if err != nil {
	log.Printf("read unit: %v", err)
	return
}
if data.ActionCooldown <= 0 {
	if err := bot.Travel(unit, enemyCore.Pos, nil); err != nil {
		log.Printf("travel: %v", err)
	}
}
```

Custom policies are evaluated once for every grid position. `TravelBlock` excludes a tile. `TravelPass` allows routing through it and waits if its next tile is occupied. `TravelAttack` allows routing through it and attacks its occupant when it becomes the next tile. If the target is unreachable, travel approaches the closest reachable position by Manhattan distance, breaking distance ties using path cost.
