package shared

import (
	"fmt"
)

type Position struct {
	X uint `json:"x"`
	Y uint `json:"y"`
}

func (p Position) DistanceTo(other Position) uint {
	dx := int(p.X) - int(other.X)
	dy := int(p.Y) - int(other.Y)
	return uint(absInt(dx) + absInt(dy))
}

func absInt(x int) int {
	if x < 0 {
		return -x
	}
	return x
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

func (d DepositData) String() string {
	return fmt.Sprintf("DepositData{Gems: %d}", d.Gems)
}

func (DepositData) isObjectData() {}

type BombData struct {
	Countdown uint
}

func (d BombData) String() string {
	return fmt.Sprintf("BombData{Countdown: %d}", d.Countdown)
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

func NewObject(objType *ObjectType, id *uint, X, Y *uint, hp *uint, teamId *uint, data *ObjectData) *Object {

	var obj Object = Object{}

	if objType != nil {
		obj.Type = *objType
	}
	if id != nil {
		obj.Id = *id
	}
	if X != nil && Y != nil {
		obj.Pos = Position{X: *X, Y: *Y}
	}
	if hp != nil {
		obj.Hp = int32(*hp)
	}
	if teamId != nil {
		obj.TeamId = *teamId
	}
	if data != nil {
		obj.ObjectData = *data
	}

	return &obj
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

func (o *Object) DistanceTo(pos Position) uint {
	dx := int(o.Pos.X) - int(pos.X)
	dy := int(o.Pos.Y) - int(pos.Y)
	return uint(abs(dx) + abs(dy))
}

func abs(x int) int {
	if x < 0 {
		return -x
	}
	return x
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
