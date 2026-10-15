---
title: "⚙️ StartGame"
permalink: "go_StartGame"
sidebarTitle: "⚙️ StartGame()"
---

```go
func DefaultBotConfig(teamID int, teamName string) BotConfig
func StartGame(config BotConfig, tick func(uint)) error
func Close() error
```

Import `"github.com/42core-team/go-client-lib"`; its package name is `core`.
There is no bot instance to create or pass around. Like the C library, the Go
library manages one active game per process.

```go
cfg := core.DefaultBotConfig(teamID, "My Team")
cfg.Debug = true
err := core.StartGame(cfg, func(tick uint) {
	core.CreateUnit("Warrior", "combat")
})
```

`DefaultBotConfig` reads `SERVER_IP` and `SERVER_PORT`, defaulting to
`127.0.0.1:4444`. `BotConfig.Debug` enables protocol logging.

`StartGame` requires a non-nil callback and blocks until the game completes or
an error occurs. Before each callback it updates `core.Game`. The callback
receives the elapsed tick number. Actions, getters, and debug helpers are
package-level functions; use them and read state only from the callback, not
from concurrent goroutines. Treat `core.Game` and its objects as read-only.

A second concurrent or nested `StartGame` returns `core.ErrGameAlreadyRunning`.
On return the connection closes and the active client is cleared. A later
`StartGame` starts with fresh state, queues, and debug data.

Unexpected disconnections, truncated frames, and malformed protocol data return
errors; wrapped causes remain inspectable with `errors.Is` and `errors.As`.
Observed defeat or elimination of all opposing cores completes normally.
Server action errors are printed and do not stop the loop. Startup connection
attempts retain their retry behavior.

`Close` may be called from another goroutine to stop an established connection.
It is idempotent while that connection is active, and returns
`core.ErrClientUnavailable` when no connection is active. Actions also return
that error before startup or after shutdown. Debug helpers do nothing then.

## Migration

Replace `NewBot(config)` followed by `bot.Run(tick)` with
`core.StartGame(config, tick)`. Change the callback to `func(uint)`, replace
`bot.Move(...)` and other instance calls with `core.Move(...)`, and replace
`g.ObjectsFilter(...)` with `core.ObjectsFilter(...)`. Read fields through
`core.Game`, for example `core.Game.MyTeamID`. No game or bot argument is needed
in your helper functions. Replace old `coregame` import aliases with `core` or
use the package's default name.
