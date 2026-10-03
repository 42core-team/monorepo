---
title: "⚙️ method Bot.CreateUnit(...)"
permalink: "go_CreateUnit"
sidebarTitle: "⚙️ CreateUnit()"
---

Queues a component-based unit creation request.

```go
func (b *Bot) CreateUnit(name string, components ...string)
```

An empty `name` asks the server to generate one. Component IDs must form a design accepted by the event's Unit Builder.

```go
bot.CreateUnit("Tank", "armor", "health", "health")
bot.CreateUnit("", "mobility") // generated name
```

The action may fail because of component rules, cost, spawn cooldown, or unavailable spawn positions. See [Unit Builder](documentation/unit_builder) and [UnitData](reference/go/objects/Object).
