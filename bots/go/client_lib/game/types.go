package game

import "fmt"

type ObjectType int

const (
	ObjectCore ObjectType = iota
	ObjectUnit
	ObjectDeposit
	ObjectWall
	ObjectGemPile
)

// Position contains zero-indexed grid coordinates.
type Position struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

func NewPosition(x, y uint) Position { return Position{X: x, Y: y} }

type ObjectData interface{ isObjectData() }

// UnitProperties contains the effective properties derived from a unit's components.
// BaseActionCooldown and DamageReductionPercent remain signed; the remaining effective values are non-negative.
type UnitProperties struct {
	Hp                     uint `json:"hp"`
	BaseActionCooldown     int  `json:"baseActionCooldown"`
	GemsPerCooldownStep    uint `json:"gemsPerCooldownStep"`
	MaxGems                uint `json:"maxGems"`
	DamageReductionPercent int  `json:"damageReductionPercent"`
	DamageCore             uint `json:"damageCore"`
	DamageUnit             uint `json:"damageUnit"`
	DamageObject           uint `json:"damageObject"`
	PostSpawnCoreCooldown  uint `json:"postSpawnCoreCooldown"`
}

type UnitData struct {
	TeamID         uint           `json:"teamId"`
	Gems           uint           `json:"gems"`
	ActionCooldown int            `json:"ActionCooldown"`
	Components     []string       `json:"components"`
	Properties     UnitProperties `json:"properties"`
	Name           string         `json:"name"`
}

func (*UnitData) isObjectData() {}

type CoreData struct {
	TeamID        uint `json:"teamId"`
	Gems          uint `json:"gems"`
	SpawnCooldown uint `json:"SpawnCooldown"`
}

func (*CoreData) isObjectData() {}

type DepositData struct {
	Gems uint `json:"gems"`
}

func (*DepositData) isObjectData() {}

type Object struct {
	ID         uint       `json:"id"`
	Type       ObjectType `json:"type"`
	Pos        Position   `json:"pos"`
	Hp         uint       `json:"hp"`
	TeamID     uint       `json:"teamId"`
	ObjectData ObjectData `json:"-"`
	Data       any        `json:"-"`
}

func (o *Object) IsAlive() bool { return o != nil && o.Hp > 0 }
func (o *Object) IsEnemy(myTeamID uint) bool {
	return o != nil && o.TeamID != myTeamID && o.TeamID != 0
}
func (o *Object) IsOfType(objectType ObjectType) bool { return o != nil && o.Type == objectType }

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

// UnitData returns a non-nil payload on success. Errors distinguish nil
// receivers, wrong object types, and missing or inconsistent payloads.
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

// CoreData returns a non-nil core payload on success.
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

// DepositData accepts both deposits and gem piles, which share a payload.
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
