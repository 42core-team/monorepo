---
title: "⚙️ Go debug methods"
permalink: "go_Debug"
sidebarTitle: "⚙️ Debug methods"
---

```go
func (b *Bot) AddObjectInfo(obj *game.Object, format string, args ...any)
func (b *Bot) AddObjectPathStep(unit *game.Object, pos game.Position)
```

Debug data is sent with the next action packet and then reset. Repeated info calls append formatted text; repeated path calls append positions.

```go
bot.AddObjectInfo(unit, "target: %d at (%d,%d)\n", target.ID, target.Pos.X, target.Pos.Y)
bot.AddObjectPathStep(unit, target.Pos)
```

Nil objects are ignored.
