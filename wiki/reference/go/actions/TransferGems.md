---
title: "⚙️ method Bot.TransferGems(...)"
permalink: "go_TransferGems"
sidebarTitle: "⚙️ TransferGems()"
---

```go
func (b *Bot) TransferGems(source *game.Object, targetPos game.Position, amount uint) error
```

Queues a transfer to an object at `targetPos`, or a gem drop on an empty position. Nil sources, sources other than units/cores, out-of-bounds destinations, and unavailable clients return errors. A nil error means queued locally. Resources, cooldown, ownership, and range remain server-side checks; server error messages are printed by the library. Cores do not have a unit action cooldown.

```go
data, err := carrier.UnitData()
if err != nil {
	log.Printf("read carrier: %v", err)
	return
}
if core := g.MyCore(); core != nil {
	if err := bot.TransferGems(carrier, core.Pos, data.Gems); err != nil {
		log.Printf("transfer gems: %v", err)
	}
}
```
