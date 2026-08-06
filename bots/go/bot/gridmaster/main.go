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
	bot.CreateUnit("Gridfighter", "combat", "siege", "health")

	enemyCore := g.EnemyCore()
	if enemyCore == nil {
		return
	}

	for _, unit := range g.TeamUnits() {
		if unit.GetUnitData().ActionCooldown <= 0 {
			bot.Travel(unit, enemyCore.Pos, nil)
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
