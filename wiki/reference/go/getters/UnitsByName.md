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
	data, err := scout.GetUnitData()
	if err != nil {
		log.Printf("read scout: %v", err)
		continue
	}
	if data.ActionCooldown <= 0 {
		if err := bot.Travel(scout, goal, nil); err != nil {
			log.Printf("travel: %v", err)
		}
	}
}
```
