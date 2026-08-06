---
title: "🧩 type Config"
permalink: "go_Config"
sidebarTitle: "🧩 Config"
---

`g.Config` contains the current server configuration, including grid and economy values, world generator data, core positions, and component rules.

```go
type ComponentsConfig struct {
	MaxComponentsPerUnit  uint
	UnitDefaultCost       uint
	UnitDefaultProperties UnitProperties
	Components            []Component
	InvalidConditions     []InvalidComponentCondition
}

func (c *Config) ComponentByID(id string) *Component
```

Each `Component` exposes its ID, cost, property modifications, and visualizer asset metadata.

```go
if combat := g.Config.ComponentByID("combat"); combat != nil {
	fmt.Println(combat.Cost)
}
```

`WorldGeneratorConfig` and invalid-condition expressions are `json.RawMessage` because their structures vary by generator and rule expression.
