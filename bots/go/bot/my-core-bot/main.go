package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

const teamName = "YOUR TEAM NAME HERE"

func tick(g *game.Game, bot *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	bot.CreateUnit("Warrior", "combat")

	enemyCore := opponentCore(g)
	if enemyCore == nil {
		return
	}

	for _, unit := range ownUnits(g) {
		if unit.GetUnitData().ActionCooldown <= 0 {
			bot.Travel(unit, enemyCore.Pos, nil)
		}
		bot.AddObjectInfo(unit,
			"I am a warrior! 🗡️ - I am heading for the opponent core at [%d,%d]! 🏰\n",
			enemyCore.Pos.X,
			enemyCore.Pos.Y,
		)
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: bot <team-id>")
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
