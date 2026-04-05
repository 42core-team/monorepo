---
title: "⚙️ function NewBot(...)"
permalink: "go_NewBot"
sidebarTitle: "⚙️ NewBot()"
---

## URL

https://github.com/42core-team/monorepo/blob/dev/bots/go/client_lib/bot.go

## Description

Creates a new Bot instance, initializes the connection to the server & prepares the game loop.

You only need to call `NewBot` followed by `Run` once in your code to start up the Core library, and this setup is already present in your code when you clone the repository. No need for you to do anything. 💆‍♀️🧘‍♀️😎

## Signature

```go
func DefaultBotConfig(teamID int, teamName string) BotConfig
func NewBot(config BotConfig) (*Bot, error)
func (b *Bot) Run(tick func(*game.Game, *Bot)) error
func (b *Bot) Close() error
```

## Parameters

### DefaultBotConfig
- `teamID int`: Your team ID, usually passed as a command-line argument
- `teamName string`: Name of your team

### NewBot
- `config BotConfig`: Configuration struct returned by `DefaultBotConfig`

### Run
- `tick func(*game.Game, *Bot)`: Function that will be called every game tick once new server data is available.

## Return

- `NewBot` returns `(*Bot, error)`: Returns a Bot pointer on success, or an error on failure.
- `Run` returns `error`: Returns an error if the connection fails.

## Examples

```go
package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func main() {
	teamID, _ := strconv.Atoi(os.Args[1])
	cfg := coregame.DefaultBotConfig(teamID, "FREDDATRON 6000 👾🤖💥")
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		fmt.Println("Error:", err)
		return
	}
	defer bot.Close()

	bot.Run(func(g *game.Game, b *coregame.Bot) {
		// your tick logic here
	})
}
```

## Related

/
