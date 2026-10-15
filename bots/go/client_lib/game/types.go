package game

import "fmt"

// An ObjectType identifies the kind of an [Object].
type ObjectType int

const (
	ObjectCore    ObjectType = iota // a team's base; destroying it eliminates the team
	ObjectUnit                      // a unit built from components
	ObjectDeposit                   // holds gems; leaves a gem pile when destroyed
	ObjectWall                      // a destructible obstacle
	ObjectGemPile                   // loose gems that a unit collects by attacking them
)

// A Position is a tile on the board. X and Y are zero-based and grow
// to the right and downward.
type Position struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

// NewPosition returns the position with the given coordinates.
func NewPosition(x, y uint) Position { return Position{X: x, Y: y} }

// ObjectData is the type-specific data of an [Object]. It is implemented by
// [*UnitData], [*CoreData] and [*DepositData].
type ObjectData interface{ isObjectData() }

// UnitProperties are the properties of a unit, derived from the default
// unit properties and the modifications of its components.
type UnitProperties struct {
	Hp                     uint `json:"hp"`                     // hit points at spawn
	BaseActionCooldown     int  `json:"baseActionCooldown"`     // ticks between actions, before the gem penalty; may be negative
	GemsPerCooldownStep    uint `json:"gemsPerCooldownStep"`    // carried gems that add one tick of action cooldown
	MaxGems                uint `json:"maxGems"`                // maximum number of carried gems
	DamageReductionPercent int  `json:"damageReductionPercent"` // percentage removed from incoming damage; may be outside [0, 100]
	DamageCore             uint `json:"damageCore"`             // damage dealt to cores
	DamageUnit             uint `json:"damageUnit"`             // damage dealt to units
	DamageObject           uint `json:"damageObject"`           // damage dealt to deposits and walls
	PostSpawnCoreCooldown  uint `json:"postSpawnCoreCooldown"`  // spawn cooldown the core gets after creating this unit
}

// UnitData is the data of an object of type [ObjectUnit].
type UnitData struct {
	TeamID         uint           `json:"teamId"`         // owning team
	Gems           uint           `json:"gems"`           // carried gems
	ActionCooldown int            `json:"ActionCooldown"` // ticks until the next action; ready if <= 0
	Components     []string       `json:"components"`     // IDs of the components the unit was built from
	Properties     UnitProperties `json:"properties"`     // properties derived from Components
	Name           string         `json:"name"`           // name given at creation
}

func (*UnitData) isObjectData() {}

// CoreData is the data of an object of type [ObjectCore].
type CoreData struct {
	TeamID        uint `json:"teamId"`        // owning team
	Gems          uint `json:"gems"`          // gems available to the team
	SpawnCooldown uint `json:"SpawnCooldown"` // ticks until the core can create the next unit
}

func (*CoreData) isObjectData() {}

// DepositData is the data of an object of type [ObjectDeposit] or
// [ObjectGemPile].
type DepositData struct {
	Gems uint `json:"gems"` // gems held by the deposit or gem pile
}

func (*DepositData) isObjectData() {}

// An Object is an entity on the board.
//
// The type-specific data in ObjectData should be accessed through
// [Object.UnitData], [Object.CoreData] and [Object.DepositData], which check
// the object type.
type Object struct {
	ID         uint       `json:"id"`     // unique ID, stable for the object's lifetime
	Type       ObjectType `json:"type"`   // kind of object
	Pos        Position   `json:"pos"`    // current tile
	Hp         uint       `json:"hp"`     // remaining hit points
	TeamID     uint       `json:"teamId"` // owning team, or 0 if neutral
	ObjectData ObjectData `json:"-"`      // type-specific data
	Data       any        `json:"-"`      // not used by the library; free to store whatever you want
}

// IsAlive reports whether o is non-nil and has hit points left.
func (o *Object) IsAlive() bool { return o != nil && o.Hp > 0 }

// IsEnemy reports whether o belongs to a team other than myTeamID.
// Neutral objects, such as walls and deposits, are not enemies.
func (o *Object) IsEnemy(myTeamID uint) bool {
	return o != nil && o.TeamID != myTeamID && o.TeamID != 0
}

// IsOfType reports whether o is non-nil and of type objectType.
func (o *Object) IsOfType(objectType ObjectType) bool { return o != nil && o.Type == objectType }

// IsFriendly reports whether o is a unit or core that belongs to team
// myTeamID.
func (o *Object) IsFriendly(myTeamID uint) bool {
	if o == nil {
		return false
	}
	switch o.Type {
	case ObjectUnit:
		data, err := o.UnitData()
		return err == nil && data.TeamID == myTeamID
	case ObjectCore:
		data, err := o.CoreData()
		return err == nil && data.TeamID == myTeamID
	}
	return false
}

// UnitData returns the unit data of o. It returns an error wrapping
// [ErrNilObject] if o is nil, [ErrWrongObjectType] if o is not a unit,
// and [ErrInvalidObjectData] if the data is missing.
func (o *Object) UnitData() (*UnitData, error) {
	if o == nil {
		return nil, ErrNilObject
	}
	if o.Type != ObjectUnit {
		return nil, fmt.Errorf("object %d: expected unit, got type %d: %w", o.ID, o.Type, ErrWrongObjectType)
	}
	data, ok := o.ObjectData.(*UnitData)
	if !ok || data == nil {
		return nil, fmt.Errorf("unit %d: %w", o.ID, ErrInvalidObjectData)
	}
	return data, nil
}

// CoreData returns the core data of o. It returns an error wrapping
// [ErrNilObject] if o is nil, [ErrWrongObjectType] if o is not a core,
// and [ErrInvalidObjectData] if the data is missing.
func (o *Object) CoreData() (*CoreData, error) {
	if o == nil {
		return nil, ErrNilObject
	}
	if o.Type != ObjectCore {
		return nil, fmt.Errorf("object %d: expected core, got type %d: %w", o.ID, o.Type, ErrWrongObjectType)
	}
	data, ok := o.ObjectData.(*CoreData)
	if !ok || data == nil {
		return nil, fmt.Errorf("core %d: %w", o.ID, ErrInvalidObjectData)
	}
	return data, nil
}

// DepositData returns the deposit data of o, which must be a deposit or a
// gem pile. It returns an error wrapping [ErrNilObject] if o is nil,
// [ErrWrongObjectType] if o is of another type, and [ErrInvalidObjectData]
// if the data is missing.
func (o *Object) DepositData() (*DepositData, error) {
	if o == nil {
		return nil, ErrNilObject
	}
	if o.Type != ObjectDeposit && o.Type != ObjectGemPile {
		return nil, fmt.Errorf("object %d: expected deposit or gem pile, got type %d: %w", o.ID, o.Type, ErrWrongObjectType)
	}
	data, ok := o.ObjectData.(*DepositData)
	if !ok || data == nil {
		return nil, fmt.Errorf("object %d: %w", o.ID, ErrInvalidObjectData)
	}
	return data, nil
}
