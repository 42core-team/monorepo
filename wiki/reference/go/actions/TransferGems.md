---
title: "⚙️ method Bot.TransferGems(...)"
permalink: "go_TransferGems"
sidebarTitle: "⚙️ TransferGems()"
---

```go
func (b *Bot) TransferGems(source *game.Object, targetPos game.Position, amount uint)
```

Transfers gems to an object at `targetPos`, or drops a gem pile on an empty position. A nil source queues nothing. Units must be ready; cores do not have a unit action cooldown.

```go
bot.TransferGems(carrier, g.MyCore().Pos, carrier.GetUnitData().Gems)
```
