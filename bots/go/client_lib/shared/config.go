package shared

import (
	"encoding/json"
	"fmt"
	"strings"
)

type BuildType int

const (
	BuildTypeNone BuildType = iota
	BuildTypeWall
	BuildTypeBomb
)

func (b *BuildType) UnmarshalJSON(data []byte) error {
	var s string
	if err := json.Unmarshal(data, &s); err != nil {
		return json.Unmarshal(data, (*int)(b))
	}
	switch strings.ToLower(s) {
	case "none":
		*b = BuildTypeNone
	case "wall":
		*b = BuildTypeWall
	case "bomb":
		*b = BuildTypeBomb
	default:
		*b = BuildTypeNone
	}
	return nil
}

type UnitConfig struct {
	Name                   string    `json:"name"`
	UnitType               UnitType  `json:"unitType"`
	Cost                   uint      `json:"cost"`
	Hp                     uint      `json:"hp"`
	BaseActionCooldown     uint      `json:"baseActionCooldown"`
	MaxActionCooldown      uint      `json:"maxActionCooldown"`
	BalancePerCooldownStep uint      `json:"balancePerCooldownStep"`
	DamageCore             uint      `json:"damageCore"`
	DamageUnit             uint      `json:"damageUnit"`
	DamageDeposit          uint      `json:"damageDeposit"`
	DamageWall             uint      `json:"damageWall"`
	DamageBomb             uint      `json:"damageBomb"`
	BuildType              BuildType `json:"buildType"`
}

type Config struct {
	GridSize          uint         `json:"gridSize"`
	IdleIncome        uint         `json:"idleIncome"`
	IdleIncomeTimeout uint         `json:"idleIncomeTimeOut"`
	DepositHp         uint         `json:"depositHp"`
	DepositIncome     uint         `json:"depositIncome"`
	GemPileIncome     uint         `json:"gemPileIncome"`
	CoreHp            uint         `json:"coreHp"`
	CoreSpawnCooldown uint         `json:"coreSpawnCooldown"`
	InitialBalance    uint         `json:"initialBalance"`
	WallHp            uint         `json:"wallHp"`
	WallBuildCost     uint         `json:"wallBuildCost"`
	BombCountdown     uint         `json:"bombCountdown"`
	BombThrowCost     uint         `json:"bombThrowCost"`
	BombReach         uint         `json:"bombReach"`
	BombDamageCore    uint         `json:"bombDamageCore"`
	BombDamageUnit    uint         `json:"bombDamageUnit"`
	BombDamageDeposit uint         `json:"bombDamageDeposit"`
	Units             []UnitConfig `json:"units"`
}

type Game struct {
	ElapsedTicks uint
	Config       Config
	MyTeamId     uint
	Objects      []Object
}

func (game *Game) GetObjectById(id uint) (*Object, error) {
	for i := range game.Objects {
		if game.Objects[i].Id == id {
			return &game.Objects[i], nil
		}
	}
	return nil, fmt.Errorf("object with id %d not found", id)
}

func (game *Game) GetMyCore() *Object {
	for i := range game.Objects {
		if game.Objects[i].IsOfType(ObjectCore) && game.Objects[i].IsAlly(game.MyTeamId) {
			return &game.Objects[i]
		}
	}
	return nil
}

func (game *Game) IsPositionWalkable(pos Position) bool {
	if pos.X >= game.Config.GridSize || pos.Y >= game.Config.GridSize {
		return false
	}
	for i := range game.Objects {
		if game.Objects[i].Pos == pos && (game.Objects[i].IsOfType(ObjectWall) || game.Objects[i].IsOfType(ObjectCore)) {
			return false
		}
	}
	return true
}

func (game *Game) GetEnemyCore() *Object {
	for i := range game.Objects {
		if game.Objects[i].IsOfType(ObjectCore) && game.Objects[i].IsEnemy(game.MyTeamId) {
			return &game.Objects[i]
		}
	}
	return nil
}

func (game *Game) GetObjectFromPosition(pos Position) *Object {
	for i := range game.Objects {
		if game.Objects[i].Pos == pos {
			return &game.Objects[i]
		}
	}
	return nil
}

func (game *Game) GetTeamUnits() []*Object {
	var units []*Object

	for i := range game.Objects {
		if game.Objects[i].IsAlly(game.MyTeamId) && game.Objects[i].IsOfType(ObjectUnit) {
			units = append(units, &game.Objects[i])
		}
	}

	return units
}

func (game *Game) GetObjectsFromFilter(filter func(object *Object) bool) []*Object {
	var filtered []*Object
	for i := range game.Objects {
		if filter(&game.Objects[i]) {
			filtered = append(filtered, &game.Objects[i])
		}
	}
	return filtered
}

func (game *Game) GetObjectFromFilterNearest(pos Position, filter func(object *Object) bool) *Object {
	nearest := &game.Objects[0]
	minDistance := uint(0)

	for i := range game.Objects {
		if filter(&game.Objects[i]) {
			distance := game.Objects[i].DistanceTo(pos)
			if minDistance == 0 || distance < minDistance {
				minDistance = distance
				nearest = &game.Objects[i]
			}
		}
	}
	return nearest
}

func (game *Game) GetUnitConfigByType(unitType UnitType) *UnitConfig {
	for i := range game.Config.Units {
		if game.Config.Units[i].UnitType == unitType {
			return &game.Config.Units[i]
		}
	}
	return nil
}

func (game *Game) Log(format string, args ...interface{}) {
	fmt.Printf(format+"\n", args...)
}
