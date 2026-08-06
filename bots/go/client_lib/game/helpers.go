package game

import (
	"fmt"
	"strings"
)

func (g *Game) Log(format string, args ...any) { fmt.Printf(format+"\n", args...) }

func (g *Game) PrintField() {
	if g.Config.GridSize == 0 {
		fmt.Println("Game config not loaded")
		return
	}
	grid := make([][]string, g.Config.GridSize)
	for y := range grid {
		grid[y] = make([]string, g.Config.GridSize)
		for x := range grid[y] {
			grid[y][x] = "."
		}
	}
	for _, obj := range g.Objects {
		if !g.IsPosValid(obj.Pos) {
			continue
		}
		symbol := map[ObjectType]string{ObjectCore: "c", ObjectUnit: "u", ObjectDeposit: "D", ObjectWall: "W", ObjectGemPile: "G"}[obj.Type]
		if obj.IsFriendly(g.MyTeamID) {
			symbol = strings.ToUpper(symbol)
		}
		grid[obj.Pos.Y][obj.Pos.X] = symbol
	}
	fmt.Printf("Tick: %d\n", g.ElapsedTicks)
	for _, row := range grid {
		fmt.Println(strings.Join(row, " "))
	}
}

func (g *Game) PrintObject(obj *Object) {
	if obj == nil {
		fmt.Println("Object is nil")
		return
	}
	fmt.Printf("ID: %d\nType: %s\nPosition: (%d, %d)\nHP: %d\n", obj.ID, objectTypeToString(obj.Type), obj.Pos.X, obj.Pos.Y, obj.Hp)
	if data := obj.GetCoreData(); data != nil {
		fmt.Printf("Team ID: %d\nGems: %d\nSpawn Cooldown: %d\n", data.TeamID, data.Gems, data.SpawnCooldown)
	}
	if data := obj.GetUnitData(); data != nil {
		fmt.Printf("Name: %s\nTeam ID: %d\nGems: %d\nAction Cooldown: %d\nComponents: %s\n", data.Name, data.TeamID, data.Gems, data.ActionCooldown, strings.Join(data.Components, ", "))
	}
	if data := obj.GetDepositData(); data != nil {
		fmt.Printf("Gems: %d\n", data.Gems)
	}
}

func (g *Game) PrintObjects(objects []*Object) {
	for _, object := range objects {
		fmt.Println("---")
		g.PrintObject(object)
	}
}

func (g *Game) PrintConfig() {
	fmt.Printf("Game Config:\n  Grid Size: %d\n  Idle Income: %d\n  Max Unit Standing Cooldown: %d\n  Components Per Unit: %d\n", g.Config.GridSize, g.Config.IdleIncome, g.Config.MaxUnitStandingCooldown, g.Config.Components.MaxComponentsPerUnit)
	for _, component := range g.Config.Components.Components {
		fmt.Printf("  Component %s (cost %d)\n", component.ID, component.Cost)
	}
}

func objectTypeToString(objectType ObjectType) string {
	switch objectType {
	case ObjectCore:
		return "Core"
	case ObjectUnit:
		return "Unit"
	case ObjectDeposit:
		return "Deposit"
	case ObjectWall:
		return "Wall"
	case ObjectGemPile:
		return "GemPile"
	default:
		return "Unknown"
	}
}
