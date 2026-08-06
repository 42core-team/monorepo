package game

import "encoding/json"

type ComponentPropertyModification struct {
	Name         string `json:"name"`
	Modification int    `json:"modification"`
}

type Component struct {
	ID                         string                          `json:"id"`
	Properties                 []ComponentPropertyModification `json:"properties"`
	Cost                       uint                            `json:"cost"`
	VisualizerAssetPrioritized bool                            `json:"visualizer_asset_prioritized"`
	VisualizerAssetPath        string                          `json:"visualizer_asset_path"`
}

type InvalidComponentCondition struct {
	Message   string          `json:"message"`
	Condition json.RawMessage `json:"condition"`
}

type ComponentsConfig struct {
	MaxComponentsPerUnit  uint                        `json:"maxComponentsPerUnit"`
	UnitDefaultCost       uint                        `json:"unitDefaultCost"`
	UnitDefaultProperties UnitProperties              `json:"unitDefaultProperties"`
	Components            []Component                 `json:"components"`
	InvalidConditions     []InvalidComponentCondition `json:"invalidConditions"`
}

type Config struct {
	GridSize                uint             `json:"gridSize"`
	Seed                    string           `json:"seed,omitempty"`
	IdleIncome              uint             `json:"idleIncome"`
	IdleIncomeTimeout       uint             `json:"idleIncomeTimeOut"`
	MaxUnitStandingCooldown uint             `json:"maxUnitStandingCooldown"`
	DepositHp               uint             `json:"depositHp"`
	DepositIncome           uint             `json:"depositIncome"`
	GemPileIncome           uint             `json:"gemPileIncome"`
	CoreHp                  uint             `json:"coreHp"`
	InitialGems             uint             `json:"initialGems"`
	WallHp                  uint             `json:"wallHp"`
	WorldGenerator          string           `json:"worldGenerator"`
	WorldGeneratorConfig    json.RawMessage  `json:"worldGeneratorConfig"`
	Components              ComponentsConfig `json:"components"`
	CorePositions           []Position       `json:"corePositions"`
}

func (c *Config) ComponentByID(id string) *Component {
	for i := range c.Components.Components {
		if c.Components.Components[i].ID == id {
			return &c.Components.Components[i]
		}
	}
	return nil
}
