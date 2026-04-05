package game

import (
	"fmt"
	"strings"
)

func (g *Game) Log(format string, args ...any) {
	fmt.Printf(format+"\n", args...)
}

func (g *Game) PrintField() {
	if g.Config.GridSize == 0 {
		fmt.Println("Game config not loaded")
		return
	}

	grid := make([][]string, g.Config.GridSize)
	for i := range grid {
		grid[i] = make([]string, g.Config.GridSize)
		for j := range grid[i] {
			grid[i][j] = "."
		}
	}

	for _, obj := range g.Objects {
		if obj.Pos.X < g.Config.GridSize && obj.Pos.Y < g.Config.GridSize {
			symbol := "."
			switch obj.Type {
			case ObjectCore:
				if data := obj.GetCoreData(); data != nil {
					if data.TeamID == g.MyTeamID {
						symbol = "C"
					} else {
						symbol = "c"
					}
				}
			case ObjectUnit:
				if data := obj.GetUnitData(); data != nil {
					if data.TeamID == g.MyTeamID {
						symbol = "U"
					} else {
						symbol = "u"
					}
				}
			case ObjectDeposit:
				symbol = "D"
			case ObjectWall:
				symbol = "W"
			case ObjectGemPile:
				symbol = "G"
			case ObjectBomb:
				symbol = "B"
			}
			grid[obj.Pos.Y][obj.Pos.X] = symbol
		}
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

	fmt.Println("---")
	fmt.Printf("ID: %d\n", obj.ID)
	fmt.Printf("Type: %s\n", objectTypeToString(obj.Type))
	fmt.Printf("Position: (%d, %d)\n", obj.Pos.X, obj.Pos.Y)
	fmt.Printf("HP: %d\n", obj.Hp)

	switch obj.Type {
	case ObjectCore:
		if data := obj.GetCoreData(); data != nil {
			fmt.Printf("Team ID: %d\n", data.TeamID)
			fmt.Printf("Gems: %d\n", data.Gems)
			fmt.Printf("Spawn Cooldown: %d\n", data.SpawnCooldown)
		}
	case ObjectUnit:
		if data := obj.GetUnitData(); data != nil {
			fmt.Printf("Unit Type: %d\n", data.UnitType)
			fmt.Printf("Team ID: %d\n", data.TeamID)
			fmt.Printf("Gems: %d\n", data.Gems)
			if data.ActionCooldown != nil {
				fmt.Printf("Action Cooldown: %d\n", *data.ActionCooldown)
			}
		}
	case ObjectDeposit, ObjectGemPile:
		if data := obj.GetDepositData(); data != nil {
			fmt.Printf("Gems: %d\n", data.Gems)
		}
	case ObjectBomb:
		if data := obj.GetBombData(); data != nil {
			fmt.Printf("Countdown: %d\n", data.Countdown)
		}
	}
}

func (g *Game) PrintObjects(objs []*Object) {
	for _, obj := range objs {
		g.PrintObject(obj)
		fmt.Println("---")
	}
}

func (g *Game) PrintConfig() {
	fmt.Println("Game Config:")
	fmt.Printf("  Grid Size: %d\n", g.Config.GridSize)
	fmt.Printf("  Idle Income: %d\n", g.Config.IdleIncome)
	fmt.Printf("  Idle Income Timeout: %d\n", g.Config.IdleIncomeTimeout)
	fmt.Printf("  Deposit HP: %d\n", g.Config.DepositHp)
	fmt.Printf("  Deposit Income: %d\n", g.Config.DepositIncome)
	fmt.Printf("  Gem Pile Income: %d\n", g.Config.GemPileIncome)
	fmt.Printf("  Core HP: %d\n", g.Config.CoreHp)
	fmt.Printf("  Core Spawn Cooldown: %d\n", g.Config.CoreSpawnCooldown)
	fmt.Printf("  Initial Balance: %d\n", g.Config.InitialBalance)
	fmt.Printf("  Wall HP: %d\n", g.Config.WallHp)
	fmt.Printf("  Wall Build Cost: %d\n", g.Config.WallBuildCost)
	fmt.Printf("  Bomb Countdown: %d\n", g.Config.BombCountdown)
	fmt.Printf("  Bomb Throw Cost: %d\n", g.Config.BombThrowCost)
	fmt.Printf("  Bomb Reach: %d\n", g.Config.BombReach)
	fmt.Printf("  Bomb Damage Core: %d\n", g.Config.BombDamageCore)
	fmt.Printf("  Bomb Damage Unit: %d\n", g.Config.BombDamageUnit)
	fmt.Printf("  Bomb Damage Deposit: %d\n", g.Config.BombDamageDeposit)

	for _, unit := range g.Config.Units {
		g.PrintUnitConfig(unit.UnitType)
	}
}

func (g *Game) PrintUnitConfig(unitType UnitType) {
	unit := g.Config.GetUnitConfig(unitType)
	if unit == nil {
		fmt.Printf("Unit type %d not found\n", unitType)
		return
	}

	fmt.Printf("Unit %s (%d) Config:\n", unit.Name, unitType)
	fmt.Printf("  Cost: %d\n", unit.Cost)
	fmt.Printf("  HP: %d\n", unit.Hp)
	fmt.Printf("  Damage Core: %d\n", unit.DamageCore)
	fmt.Printf("  Damage Unit: %d\n", unit.DamageUnit)
	fmt.Printf("  Damage Deposit: %d\n", unit.DamageDeposit)
	fmt.Printf("  Damage Wall: %d\n", unit.DamageWall)
	fmt.Printf("  Damage Bomb: %d\n", unit.DamageBomb)
	fmt.Printf("  Build Type: %s\n", buildTypeToString(unit.BuildType))
	fmt.Printf("  Base Action Cooldown: %d\n", unit.BaseActionCooldown)
	fmt.Printf("  Max Action Cooldown: %d\n", unit.MaxActionCooldown)
	fmt.Printf("  Balance Per Cooldown Step: %d\n", unit.BalancePerCooldown)
}

func objectTypeToString(t ObjectType) string {
	switch t {
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
	case ObjectBomb:
		return "Bomb"
	default:
		return "Unknown"
	}
}

func buildTypeToString(t BuildType) string {
	switch t {
	case BuildTypeNone:
		return "None"
	case BuildTypeWall:
		return "Wall"
	case BuildTypeBomb:
		return "Bomb"
	default:
		return "Unknown"
	}
}
