package shared

type Position struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

func NewPosition(x uint, y uint) Position {
	return Position{X: x, Y: y}
}

type ObjectData interface {
	isObjectData()
}

type CoreData struct {
	TeamId        uint `json:"team_id"`
	Gems          uint `json:"gems"`
	SpawnCooldown uint `json:"spawn_cooldown"`
}

func (CoreData) isObjectData() {}

type UnitData struct {
	UnitType       UnitType `json:"type"`
	TeamId         uint     `json:"-"`
	Gems           *uint    `json:"gems,omitempty"`
	ActionCooldown *uint    `json:"ActionCooldown,omitempty"`
}

func (UnitData) isObjectData() {}

type DepositData struct {
	Gems uint `json:"gems"`
}

func (DepositData) isObjectData() {}

type BombData struct {
	Countdown uint
}

func (BombData) isObjectData() {}

type Object struct {
	Type       ObjectType
	Data       *any
	Id         uint
	Pos        Position
	Hp         int32
	TeamId     uint
	ObjectData ObjectData
}

func NewObject(objType ObjectType, id uint, pos Position, hp int32, data ObjectData) *Object {
	return &Object{
		Type:       objType,
		Id:         id,
		Pos:        pos,
		Hp:         hp,
		ObjectData: data,
	}
}

func (o *Object) IsOfType(objectType ObjectType) bool {
	return o.Type == objectType
}

func (o *Object) Tick() {
	switch data := o.ObjectData.(type) {
	case UnitData:
		if data.ActionCooldown == nil {
			data.ActionCooldown = new(uint(0))
		}
		if *data.ActionCooldown > 0 {
			*data.ActionCooldown--
		}
		o.ObjectData = data
	}
}

func (o *Object) GetUnitData() (UnitData, bool) {
	data, ok := o.ObjectData.(UnitData)
	return data, ok
}

func (o *Object) GetCoreData() (CoreData, bool) {
	data, ok := o.ObjectData.(CoreData)
	return data, ok
}

func (o *Object) GetDepositData() (DepositData, bool) {
	data, ok := o.ObjectData.(DepositData)
	return data, ok
}

func (o *Object) GetBombData() (BombData, bool) {
	data, ok := o.ObjectData.(BombData)
	return data, ok
}

func (o *Object) IsAlive() bool {
	return o.Hp > 0
}

func (o *Object) IsEnemy(teamId uint) bool {
	if o.TeamId == 0 {
		return false
	}
	return o.TeamId != teamId
}

func (o *Object) IsAlly(teamId uint) bool {
	if o.TeamId == 0 {
		return false
	}
	return o.TeamId == teamId
}

func (o *Object) IsReadyForAction() bool {
	if o.Hp <= 0 {
		return false
	}
	switch data := o.ObjectData.(type) {
	case UnitData:
		return data.ActionCooldown == nil || *data.ActionCooldown == 0
	default:
		return true
	}
}

//func (o *Object) Update(updatedData *Object) error {
//	if o.Id != updatedData.Id {
//		return fmt.Errorf("cannot update object with different id: %d vs %d", o.Id, updatedData.Id)
//	}
//	o.Hp = updatedData.Hp
//	o.Pos = updatedData.Pos
//	o.ObjectData = updatedData.ObjectData
//	return nil
//}
