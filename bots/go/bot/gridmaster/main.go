package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

const teamName = "Gridmaster"

func tick(g *game.Game, bot *coregame.Bot) {
	if err := bot.CreateUnit("Gridfighter", "combat", "siege", "health"); err != nil {
		fmt.Fprintf(os.Stderr, "Queue fighter: %v\n", err)
	}

	enemyCore := opponentCore(g)
	if enemyCore == nil {
		return
	}

	for _, unit := range ownUnits(g) {
		data, err := unit.UnitData()
		if err != nil {
			fmt.Fprintf(os.Stderr, "Read unit: %v\n", err)
			continue
		}
		if data.ActionCooldown <= 0 {
			if err := bot.Travel(unit, enemyCore.Pos, nil); err != nil {
				fmt.Fprintf(os.Stderr, "Travel: %v\n", err)
			}
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: gridmaster <team-id>")
		os.Exit(1)
	}

	teamID, err := strconv.Atoi(os.Args[1])
	if err != nil {
		fmt.Printf("Invalid team ID: %v\n", err)
		os.Exit(1)
	}

	cfg := coregame.DefaultBotConfig(teamID, teamName)
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		fmt.Printf("Error creating bot: %v\n", err)
		return
	}

	if err := bot.Run(tick); err != nil {
		fmt.Printf("Bot error: %v\n", err)
	}
}
