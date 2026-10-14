---
title: "⚙️ method Bot.CreateUnit(...)"
permalink: "go_CreateUnit"
sidebarTitle: "⚙️ CreateUnit()"
---

Queues a component-based unit creation request.

```go
func (b *Bot) CreateUnit(name string, components ...string) error
```

An empty `name` asks the server to generate one. Component IDs must form a design accepted by the event's Unit Builder.

```go
if err := bot.CreateUnit("Tank", "armor", "health", "health"); err != nil {
	log.Printf("create tank: %v", err)
}
if err := bot.CreateUnit("", "mobility"); err != nil { // generated name
	log.Printf("create scout: %v", err)
}
```

Returns `coregame.ErrClientUnavailable` if the client is unavailable. A nil error means queued locally. The server may later reject the action because of component rules, cost, spawn cooldown, or unavailable spawn positions; server error messages are printed by the library. See [Unit Builder](documentation/unit_builder) and [UnitData](reference/go/objects/Object).
