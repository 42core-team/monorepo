package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

func tick(g *game.Game, bot *coregame.Bot) {
	fmt.Printf("-----> [⚡️ TICK %d 🔥]\n", g.ElapsedTicks)

	for _, rejection := range g.ActionRejections {
		fmt.Fprintf(os.Stderr, "Tick %d: server rejected action: %v\n", rejection.Tick, rejection)
	}
	if err := bot.CreateUnit("Warrior", "combat"); err != nil {
		fmt.Fprintf(os.Stderr, "Queue warrior: %v\n", err)
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

	cfg := coregame.DefaultBotConfig(teamID, "My CORE Bot")
	bot, err := coregame.NewBot(cfg)
	if err != nil {
		fmt.Printf("Error creating bot: %v\n", err)
		return
	}

	if err := bot.Run(tick); err != nil {
		fmt.Printf("Bot error: %v\n", err)
	}
}
