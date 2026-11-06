---
title: "🧩 type UnitProperties"
permalink: "go_UnitProperties"
sidebarTitle: "🧩 UnitProperties"
---

`UnitProperties` contains effective values derived from a unit's components:

```go
type UnitProperties struct {
	Hp, GemsPerCooldownStep, MaxGems                uint
	BaseActionCooldown, DamageReductionPercent      int
	DamageCore, DamageUnit, DamageObject             uint
	PostSpawnCoreCooldown                            uint
}
```

Call `data, err := unit.UnitData()`, handle the error, then read `data.Properties`. `BaseActionCooldown` and `DamageReductionPercent` are signed; the remaining effective values are non-negative. Component modifications in the configuration remain signed. See the [Unit Builder property guide](documentation/unit_builder).
