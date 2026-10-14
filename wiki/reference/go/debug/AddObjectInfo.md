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
data, err := unit.GetUnitData()
if err != nil {
	log.Printf("read unit: %v", err)
	return
}
bot.AddObjectInfo(unit, "carrying %d gems\n", data.Gems)
```
