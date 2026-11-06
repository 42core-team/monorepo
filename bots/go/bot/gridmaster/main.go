package main

import (
	"fmt"
	"os"
	"strconv"

	"github.com/42core-team/go-client-lib"
)

const teamName = "Gridmaster"

func tick(elapsedTicks uint) {
	if err := core.CreateUnit("Gridfighter", "combat", "siege", "health"); err != nil {
		fmt.Fprintf(os.Stderr, "Queue fighter: %v\n", err)
	}

	enemyCore := opponentCore()
	if enemyCore == nil {
		return
	}

	for _, unit := range ownUnits() {
		data, err := unit.UnitData()
		if err != nil {
			fmt.Fprintf(os.Stderr, "Read unit: %v\n", err)
			continue
		}
		if data.ActionCooldown <= 0 {
			if err := core.Travel(unit, enemyCore.Pos, nil); err != nil {
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

	cfg := core.DefaultBotConfig(teamID, teamName)
	if err := core.StartGame(cfg, tick); err != nil {
		fmt.Printf("Bot error: %v\n", err)
	}
}
