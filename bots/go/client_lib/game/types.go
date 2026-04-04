package game

type ObjectType int

const (
	ObjectCore ObjectType = iota
	ObjectUnit
	ObjectDeposit
	ObjectWall
	ObjectGemPile
	ObjectBomb
)

type UnitType int

const (
	UnitWarrior UnitType = iota
	UnitMiner
	UnitCarrier
	UnitTank
)

type Position struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

func NewPosition(x, y uint) Position {
	return Position{X: x, Y: y}
}

type ObjectData interface {
	isObjectData()
}

type UnitData struct {
	UnitType       UnitType `json:"unit_type"`
	TeamID         uint     `json:"teamId"`
	Gems           *uint    `json:"gems,omitempty"`
	ActionCooldown *uint    `json:"ActionCooldown,omitempty"`
}

func (UnitData) isObjectData() {}

type CoreData struct {
	TeamID        uint `json:"teamId"`
	Gems          uint `json:"gems"`
	SpawnCooldown uint `json:"SpawnCooldown"`
}

func (CoreData) isObjectData() {}

type DepositData struct {
	Gems uint `json:"gems"`
}

func (DepositData) isObjectData() {}

type BombData struct {
	Countdown uint `json:"countdown"`
}

func (BombData) isObjectData() {}

type Object struct {
	ID         uint       `json:"id"`
	Type       ObjectType `json:"type"`
	Pos        Position   `json:"pos"`
	Hp         int32      `json:"hp"`
	TeamID     uint       `json:"teamId"`
	ObjectData ObjectData `json:"-"`
	Data       any        `json:"-"`
}

func (o *Object) IsAlive() bool {
	return o.Hp > 0
}

func (o *Object) IsEnemy(myTeamID uint) bool {
	return o.TeamID != myTeamID && o.TeamID != 0
}

func (o *Object) IsOfType(objType ObjectType) bool {
	return o.Type == objType
}

func (o *Object) IsFriendly(myTeamID uint) bool {
	if o.Type == ObjectUnit {
		if data, ok := o.ObjectData.(UnitData); ok {
			return data.TeamID == myTeamID
		}
	}
	if o.Type == ObjectCore {
		if data, ok := o.ObjectData.(CoreData); ok {
			return data.TeamID == myTeamID
		}
	}
	return false
}

func (o *Object) GetUnitData() *UnitData {
	if data, ok := o.ObjectData.(UnitData); ok {
		return &data
	}
	return nil
}

func (o *Object) GetCoreData() *CoreData {
	if data, ok := o.ObjectData.(CoreData); ok {
		return &data
	}
	return nil
}

func (o *Object) GetDepositData() *DepositData {
	if data, ok := o.ObjectData.(DepositData); ok {
		return &data
	}
	return nil
}

func (o *Object) GetBombData() *BombData {
	if data, ok := o.ObjectData.(BombData); ok {
		return &data
	}
	return nil
}
