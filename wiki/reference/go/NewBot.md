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

`Run` requires a non-nil callback and blocks until the game completes or an error occurs. Unexpected disconnections, truncated frames, and malformed protocol data return errors; wrapped causes remain inspectable with `errors.Is` and `errors.As`. Observed defeat or elimination of all opposing cores completes normally. Server action error messages are printed by the library and do not stop the loop.

`Close` is idempotent for an initialized bot. Queuing actions or calling `Run` on an unavailable/closed bot returns `coregame.ErrClientUnavailable`. Connection establishment retains its startup retry behavior.
