---
title: "⚙️ method Bot.Attack(...)"
permalink: "go_Attack"
sidebarTitle: "⚙️ Attack()"
---

```go
func (b *Bot) Attack(attacker, target *game.Object)
```

Queues an attack from a unit against an adjacent object. Damage comes from the attacker's current component-derived properties. Nil inputs queue nothing.

```go
if target := g.ObjectAtPos(next); target != nil {
	bot.Attack(unit, target)
}
```

Units can attack only when their action cooldown is zero or negative.
