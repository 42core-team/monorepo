package main

import (
	"fmt"
	"os"
	"strconv"

	coregame "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/game"
)

const teamName = "Tudn"

func tick(g *game.Game, bot *coregame.Bot) {
	g.Log("Tick %d", g.ElapsedTicks)

	if g.ElapsedTicks%3 == 0 {
		_ = bot.CreateUnit(game.UnitMiner)
	} else {
		_ = bot.CreateUnit(game.UnitWarrior)
	}

	var enemyCore = g.GetEnemyCore()
	fmt.Printf("Number of team units: %d\n", len(g.GetTeamUnits()))
	fmt.Printf("Number of enemy units: %d\n", len(g.GetEnemyUnits()))
	for _, obj := range g.GetTeamUnits() {
		if !obj.IsAlive() {
			continue
		}

		var closestEnemy = g.GetObjectFromFilterNearest(obj.Pos, func(object *game.Object) bool {
			return object.IsEnemy(obj.TeamId) && object.IsAlive() && (object.IsOfType(game.ObjectUnit) || object.IsOfType(game.ObjectCore))
		})

		// fmt.Printf("Enemy core pos: %v\n", enemyCore.Pos)
		// fmt.Printf("closest enemy: %v\n", closestEnemy)

		if closestEnemy != nil && closestEnemy.IsAlive() {
			// fmt.Println("Moving to closest enemy")
			pos := bot.SimplePathfind(obj, closestEnemy.Pos)
			bot.Move(obj, pos)
			bot.Attack(obj, closestEnemy)
		} else {
			// fmt.Println("Moving to enemy core")
			pos := bot.SimplePathfind(obj, enemyCore.Pos)
			bot.Move(obj, pos)
			bot.Attack(obj, enemyCore)
		}
	}
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: my-core-bot <team-id>")
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
