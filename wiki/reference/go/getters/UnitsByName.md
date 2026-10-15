---
title: "⚙️ function UnitsByName(...)"
permalink: "go_UnitsByName"
sidebarTitle: "⚙️ UnitsByName()"
---

```go
func UnitsByName(name string) []*game.Object
func UnitsByNameCount(name string) uint
```

Returns or counts your units with the custom or generated name. Opposing units are excluded. An empty name or no matches returns nil/zero.

```go
for _, scout := range core.UnitsByName("Scout") {
	data, err := scout.UnitData()
	if err != nil {
		log.Printf("read scout: %v", err)
		continue
	}
	if data.ActionCooldown <= 0 {
		if err := core.Travel(scout, goal, nil); err != nil {
			log.Printf("travel: %v", err)
		}
	}
}
```
