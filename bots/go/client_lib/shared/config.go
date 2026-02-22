package shared

import "fmt"

type BuildType int

const (
	BuildTypeNone BuildType = iota
	BuildTypeWall
	BuildTypeBomb
)

type UnitConfig struct {
	Name                   string
	UnitType               UnitType
	Cost                   uint
	Hp                     uint
	BaseActionCooldown     uint
	MaxActionCooldown      uint
	BalancePerCooldownStep uint
	DamageCore             uint
	DamageUnit             uint
	DamageDeposit          uint
	DamageWall             uint
	DamageBomb             uint
	BuildType              BuildType
}

type Config struct {
	GridSize          uint
	IdleIncome        uint
	IdleIncomeTimeout uint
	DepositHp         uint
	DepositIncome     uint
	GemPileIncome     uint
	CoreHp            uint
	CoreSpawnCooldown uint
	InitialBalance    uint
	WallHp            uint
	WallBuildCost     uint
	BombCountdown     uint
	BombThrowCost     uint
	BombReach         uint
	BombDamageCore    uint
	BombDamageUnit    uint
	BombDamageDeposit uint
	Units             []UnitConfig
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
	fmt.Printf(format, args...)
}
