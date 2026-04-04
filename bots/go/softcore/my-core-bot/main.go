package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

const teamName = "Gridmaster"

func isEnemyCore(myTeamID uint) func(*game.Object) bool {
	return func(obj *game.Object) bool {
		if obj.Type != game.ObjectCore {
			return false
		}
		data := obj.GetCoreData()
		if data == nil {
			return false
		}
		return data.TeamID != myTeamID
	}
}

func tick(g *game.Game, bot *coregame.Bot) {
	bot.CreateUnit(game.UnitWarrior)

	enemyCore := g.NearestObject(game.Position{X: 0, Y: 0}, isEnemyCore(g.MyTeamID))
	if enemyCore == nil {
		return
	}

	for _, unit := range g.TeamUnits() {
		pos := bot.SimplePathfind(unit, enemyCore.Pos)
		bot.Move(unit, pos)
		bot.AddObjectInfo(unit, "test")
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
