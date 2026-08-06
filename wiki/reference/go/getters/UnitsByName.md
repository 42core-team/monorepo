---
title: "⚙️ method Game.UnitsByName(...)"
permalink: "go_UnitsByName"
sidebarTitle: "⚙️ UnitsByName()"
---

```go
func (g *Game) UnitsByName(name string) []*game.Object
func (g *Game) UnitsByNameCount(name string) uint
```

Returns or counts your units with the custom or generated name. Opposing units are excluded. An empty name or no matches returns nil/zero.

```go
for _, scout := range g.UnitsByName("Scout") {
	if scout.GetUnitData().ActionCooldown <= 0 {
		bot.Travel(scout, goal, nil)
	}
}
```
