package game

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
	if data := o.GetUnitData(); data != nil {
		return data.TeamID == myTeamID
	}
	if data := o.GetCoreData(); data != nil {
		return data.TeamID == myTeamID
	}
	return false
}

func (o *Object) GetUnitData() *UnitData {
	if o == nil {
		return nil
	}
	data, _ := o.ObjectData.(*UnitData)
	return data
}

func (o *Object) GetCoreData() *CoreData {
	if o == nil {
		return nil
	}
	data, _ := o.ObjectData.(*CoreData)
	return data
}

func (o *Object) GetDepositData() *DepositData {
	if o == nil {
		return nil
	}
	data, _ := o.ObjectData.(*DepositData)
	return data
}
