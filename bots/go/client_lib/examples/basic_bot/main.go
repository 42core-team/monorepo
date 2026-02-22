package main

import (
	"fmt"

	goconn "github.com/42core-team/go-client-lib"
	"github.com/42core-team/go-client-lib/shared"
)

const teamName = "GoConnBotExample"

var coreBot *goconn.CoreGameBot

func tick(game *shared.Game) {
	game.Log("Tick %d", game.ElapsedTicks)

	// Create a unit if we have enough gems
	_ = coreBot.CreateUnit(shared.UnitWarrior)

	// Move all our units
	for _, obj := range game.GetTeamUnits() {
		game.Log("Unit %d at position (%d, %d)", obj.Id, obj.Pos.X, obj.Pos.Y)
		_ = coreBot.Move(obj, shared.NewPosition(obj.Pos.X+1, obj.Pos.Y))
	}
}

func main() {
	cgb, err := goconn.NewCoreGameBot(teamName)
	if err != nil {
		fmt.Printf("Error creating bot: %v\n", err)
		return
	}
	coreBot = cgb

	if err := coreBot.Run(tick); err != nil {
		fmt.Printf("Bot error: %v\n", err)
	}
}
