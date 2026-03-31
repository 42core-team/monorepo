package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

const teamName = "GoBotExample"

func tick(g *game.Game, bot *coregame.Bot) {
	g.Log("Tick %d", g.ElapsedTicks)

	_ = bot.CreateUnit(game.UnitWarrior)

	for _, obj := range g.TeamUnits() {
		// g.Log("Unit %d at position (%d, %d)", obj.ID, obj.Pos.X, obj.Pos.Y)
		_ = bot.Move(obj, game.NewPosition(obj.Pos.X+1, obj.Pos.Y))
	}

	g.PrintField()
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
