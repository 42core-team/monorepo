---
title: "⚙️ method Bot.AddObjectInfo(...)"
permalink: "go_AddObjectInfo"
sidebarTitle: "⚙️ AddObjectInfo()"
---

```go
func (b *Bot) AddObjectInfo(obj *game.Object, format string, args ...any)
```

Appends formatted visualizer text to an object for the next packet. Nil objects are ignored.

```go
bot.AddObjectInfo(unit, "carrying %d gems\n", unit.GetUnitData().Gems)
```
