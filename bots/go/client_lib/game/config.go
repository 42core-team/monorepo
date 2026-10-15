package game

import "encoding/json"

// A ComponentPropertyModification is the amount Modification that a
// component adds to the unit property called Name, such as "hp" or
// "damageUnit". See [UnitProperties] for the available properties.
type ComponentPropertyModification struct {
	Name         string `json:"name"`
	Modification int    `json:"modification"`
}

// A Component is a building block of a unit. A unit's properties are the
// default unit properties plus the modifications of all its components.
type Component struct {
	ID                         string                          `json:"id"`                           // ID passed to core.CreateUnit
	Properties                 []ComponentPropertyModification `json:"properties"`                   // property modifications
	Cost                       uint                            `json:"cost"`                         // gems added to the unit cost
	VisualizerAssetPrioritized bool                            `json:"visualizer_asset_prioritized"` // used by the visualizer only
	VisualizerAssetPath        string                          `json:"visualizer_asset_path"`        // used by the visualizer only
}

// An InvalidComponentCondition is a rule that rejects certain combinations
// of components. Condition holds the raw rule as sent by the server, and
// Message explains why a unit matching it is rejected.
type InvalidComponentCondition struct {
	Message   string          `json:"message"`
	Condition json.RawMessage `json:"condition"`
}

// A ComponentsConfig describes how units are built from components.
type ComponentsConfig struct {
	MaxComponentsPerUnit  uint                        `json:"maxComponentsPerUnit"`  // maximum number of components in a unit
	UnitDefaultCost       uint                        `json:"unitDefaultCost"`       // gems a unit costs before component costs
	UnitDefaultProperties UnitProperties              `json:"unitDefaultProperties"` // properties before component modifications
	Components            []Component                 `json:"components"`            // available components
	InvalidConditions     []InvalidComponentCondition `json:"invalidConditions"`     // rejected component combinations
}

// A Config is the game configuration sent by the server at the start of a
// game. It does not change during the game.
type Config struct {
	GridSize                uint             `json:"gridSize"`                // width and height of the square board
	Seed                    string           `json:"seed,omitempty"`          // world generation seed, if published
	IdleIncome              uint             `json:"idleIncome"`              // gems each core receives per tick
	IdleIncomeTimeout       uint             `json:"idleIncomeTimeOut"`       // number of ticks during which idle income is paid
	MaxUnitStandingCooldown uint             `json:"maxUnitStandingCooldown"` // limit for the negative cooldown an idle unit can build up
	DepositHp               uint             `json:"depositHp"`               // initial hit points of deposits
	DepositIncome           uint             `json:"depositIncome"`           // base number of gems in a generated deposit
	GemPileIncome           uint             `json:"gemPileIncome"`           // base number of gems in a generated gem pile
	CoreHp                  uint             `json:"coreHp"`                  // initial hit points of cores
	InitialGems             uint             `json:"initialGems"`             // gems each core starts with
	WallHp                  uint             `json:"wallHp"`                  // initial hit points of walls
	WorldGenerator          string           `json:"worldGenerator"`          // name of the world generator
	WorldGeneratorConfig    json.RawMessage  `json:"worldGeneratorConfig"`    // raw world generator settings
	Components              ComponentsConfig `json:"components"`              // unit components and rules
	CorePositions           []Position       `json:"corePositions"`           // possible core positions
}

// ComponentByID returns the component with the given id, or nil if no such
// component exists. The result points into c and must not be modified.
func (c *Config) ComponentByID(id string) *Component {
	for i := range c.Components.Components {
		if c.Components.Components[i].ID == id {
			return &c.Components.Components[i]
		}
	}
	return nil
}
