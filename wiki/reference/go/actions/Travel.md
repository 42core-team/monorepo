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
func (b *Bot) Travel(unit *game.Object, target game.Position, policy TravelPolicy)
```

Pass nil for the default component-aware policy. It blocks friendly cores, waits for friendly units, estimates attack costs from effective damage and HP, and blocks gem piles when the unit has reached `MaxGems`.

Travel does not check the traveling unit's action cooldown. Check it before calling:

```go
if unit.GetUnitData().ActionCooldown <= 0 {
	bot.Travel(unit, enemyCore.Pos, nil)
}
```

Custom policies are evaluated once for every grid position. `TravelBlock` excludes a tile. `TravelPass` allows routing through it and waits if its next tile is occupied. `TravelAttack` allows routing through it and attacks its occupant when it becomes the next tile. If the target is unreachable, travel approaches the closest reachable position by Manhattan distance, breaking distance ties using path cost.
