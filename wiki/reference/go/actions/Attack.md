---
title: "⚙️ method Bot.Attack(...)"
permalink: "go_Attack"
sidebarTitle: "⚙️ Attack()"
---

```go
func (b *Bot) Attack(attacker, target *game.Object) error
```

Queues an attack from a unit against an adjacent object. Damage comes from the attacker's current component-derived properties. Nil inputs, non-unit attackers, and unavailable clients return errors. A nil error means queued locally; cooldown, ownership, range, and object existence are checked by the server. Later rejections appear in `g.ActionRejections`.

```go
if target := g.ObjectAtPos(next); target != nil {
	if err := bot.Attack(unit, target); err != nil {
		log.Printf("attack: %v", err)
	}
}
```

Units can attack only when their action cooldown is zero or negative.
