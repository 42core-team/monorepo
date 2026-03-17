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
	Name                   string   `json:"name"`
	UnitType               UnitType `json:"unitType"`
	Cost                   uint     `json:"cost"`
	Hp                     uint     `json:"hp"`
	BaseActionCooldown     uint     `json:"baseActionCooldown"`
	MaxActionCooldown      uint     `json:"maxActionCooldown"`
	BalancePerCooldownStep uint     `json:"balancePerCooldownStep"`
	DamageCore             uint     `json:"damageCore"`
	DamageUnit             uint     `json:"damageUnit"`
	DamageDeposit          uint     `json:"damageDeposit"`
	DamageWall             uint     `json:"damageWall"`
	DamageBomb             uint     `json:"damageBomb"`
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
	for _, object := range game.Objects {
		if object.Id == id {
			return &object, nil
		}
	}
	return nil, fmt.Errorf("object with id %d not found", id)
}

func (game *Game) GetObjectFromPosition(pos Position) *Object {
	return &game.Objects[0]
}

func (game *Game) GetTeamUnits() []*Object {
	var units []*Object

	for _, object := range game.Objects {
		if object.IsAlly(game.MyTeamId) {
			units = append(units, &object)
		}
	}

	return units
}

func (game *Game) GetObjectsFromFilter(filter func(object *Object) bool) []*Object {
	return []*Object{&game.Objects[0]}
}

func (game *Game) GetObjectFromFilterNearest(pos Position, filter func(object *Object) bool) *Object {
	return &game.Objects[0]
}

func (game *Game) GetUnitConfigByType(unitType UnitType) *UnitConfig {
	return &game.Config.Units[0]
}

func (game *Game) Log(format string, args ...interface{}) {
	fmt.Printf(format+"\n", args...)
}
