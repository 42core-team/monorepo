package game

import (
	"encoding/json"
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
		return err
	}
	switch strings.ToLower(s) {
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
	Name               string    `json:"name"`
	UnitType           UnitType  `json:"-"`
	Cost               uint      `json:"cost"`
	Hp                 uint      `json:"hp"`
	BaseActionCooldown uint      `json:"baseActionCooldown"`
	MaxActionCooldown  uint      `json:"maxActionCooldown"`
	BalancePerCooldown uint      `json:"balancePerCooldownStep"`
	DamageCore         uint      `json:"damageCore"`
	DamageUnit         uint      `json:"damageUnit"`
	DamageDeposit      uint      `json:"damageDeposit"`
	DamageWall         uint      `json:"damageWall"`
	DamageBomb         uint      `json:"damageBomb"`
	BuildType          BuildType `json:"buildType"`
}

type Config struct {
	GridSize          uint          `json:"gridSize"`
	IdleIncome        uint          `json:"idleIncome"`
	IdleIncomeTimeout uint          `json:"idleIncomeTimeOut"`
	DepositHp         uint          `json:"depositHp"`
	DepositIncome     uint          `json:"depositIncome"`
	GemPileIncome     uint          `json:"gemPileIncome"`
	CoreHp            uint          `json:"coreHp"`
	CoreSpawnCooldown uint          `json:"coreSpawnCooldown"`
	InitialBalance    uint          `json:"initialBalance"`
	WallHp            uint          `json:"wallHp"`
	WallBuildCost     uint          `json:"wallBuildCost"`
	BombCountdown     uint          `json:"bombCountdown"`
	BombThrowCost     uint          `json:"bombThrowCost"`
	BombReach         uint          `json:"bombReach"`
	BombDamageCore    uint          `json:"bombDamageCore"`
	BombDamageUnit    uint          `json:"bombDamageUnit"`
	BombDamageDeposit uint          `json:"bombDamageDeposit"`
	Units             []*UnitConfig `json:"units"`
}

func (c *Config) UnmarshalJSON(data []byte) error {
	type Alias Config
	aux := &struct {
		*Alias
	}{
		Alias: (*Alias)(c),
	}
	if err := json.Unmarshal(data, aux); err != nil {
		return err
	}
	for i, unit := range c.Units {
		unit.UnitType = UnitType(i)
	}
	return nil
}

func (c *Config) GetUnitConfig(unitType UnitType) *UnitConfig {
	for _, unit := range c.Units {
		if unit.UnitType == unitType {
			return unit
		}
	}
	return nil
}
