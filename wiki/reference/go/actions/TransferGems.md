---
title: "⚙️ function TransferGems(...)"
permalink: "go_TransferGems"
sidebarTitle: "⚙️ TransferGems()"
---

```go
func TransferGems(source *game.Object, targetPos game.Position, amount uint) error
```

Queues a transfer to an object at `targetPos`, or a gem drop on an empty position. Nil sources, sources other than units/cores, out-of-bounds destinations, and unavailable clients return errors. A nil error means queued locally. Resources, cooldown, ownership, and range remain server-side checks; server error messages are printed by the library. Cores do not have a unit action cooldown.

The example uses `ownCore()` from the template's `getters.go`.

```go
data, err := carrier.UnitData()
if err != nil {
	log.Printf("read carrier: %v", err)
	return
}
if own := ownCore(); own != nil {
	if err := core.TransferGems(carrier, own.Pos, data.Gems); err != nil {
		log.Printf("transfer gems: %v", err)
	}
}
```
