---
title: "⚙️ NewBot and Run"
permalink: "go_NewBot"
sidebarTitle: "⚙️ NewBot()"
---

```go
func DefaultBotConfig(teamID int, teamName string) BotConfig
func NewBot(config BotConfig) (*Bot, error)
func (b *Bot) Run(tick func(*game.Game, *Bot)) error
func (b *Bot) Close() error
func (b *Bot) Game() *game.Game
```

`DefaultBotConfig` reads `SERVER_IP` and `SERVER_PORT`, defaulting to `127.0.0.1:4444`. Set `BotConfig.Debug` to print the received config, outgoing packets, and state updates.

```go
cfg := coregame.DefaultBotConfig(teamID, "My Team")
cfg.Debug = true
bot, err := coregame.NewBot(cfg)
if err == nil {
	err = bot.Run(tick)
}
```

`Run` blocks until the connection closes or an error occurs and requires a non-nil callback.
