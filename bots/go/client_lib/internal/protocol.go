package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

type incomingObject struct {
	Id            *uint            `json:"id,omitempty"`
	Type          *game.ObjectType `json:"type,omitempty"`
	X             *uint            `json:"x,omitempty"`
	Y             *uint            `json:"y,omitempty"`
	Hp            *uint            `json:"hp,omitempty"`
	TeamId        *uint            `json:"teamId,omitempty"`
	Gems          *uint            `json:"gems,omitempty"`
	SpawnCooldown *uint            `json:"SpawnCooldown,omitempty"`
}

type incomingAction struct {
	Type     *string `json:"type,omitempty"`
	UnitId   *uint   `json:"unit_id,omitempty"`
	X        *uint   `json:"x,omitempty"`
	Y        *uint   `json:"y,omitempty"`
	TargetId *uint   `json:"target_id,omitempty"`
	UnitType *uint   `json:"unit_type,omitempty"`
}

type GameTick struct {
	Objects []incomingObject `json:"objects"`
	Actions []incomingAction `json:"actions"`
	Errors  []string         `json:"errors"`
	Tick    uint             `json:"tick"`
}

func NewGameTick(tickData string) (*GameTick, error) {
	tick := &GameTick{}
	if err := json.Unmarshal([]byte(tickData), tick); err != nil {
		return nil, fmt.Errorf("error unmarshalling game tick data: %v", err)
	}
	return tick, nil
}

func (tick *GameTick) UpdateGame(g *game.Game) {
	g.ElapsedTicks = tick.Tick

	// Apply server data
	for _, obj := range tick.Objects {
		if obj.Id == nil {
			continue
		}
		gameObject, err := g.GetObjectById(*obj.Id)
		if err != nil {
			newObject := game.NewObject(obj.Type, obj.Id, obj.X, obj.Y, obj.Hp, obj.TeamId, nil)
			newObject.ObjectData = initializeObjectData(obj)
			g.Objects = append(g.Objects, *newObject)
			continue
		}

		if obj.X != nil {
			gameObject.Pos.X = *obj.X
		}
		if obj.Y != nil {
			gameObject.Pos.Y = *obj.Y
		}
		if obj.Hp != nil {
			gameObject.Hp = int32(*obj.Hp)
		}
		if obj.TeamId != nil {
			gameObject.TeamId = *obj.TeamId
		}
		if obj.Gems != nil {
			switch data := gameObject.ObjectData.(type) {
			case game.UnitData:
				data.Gems = obj.Gems
				gameObject.ObjectData = data
			case game.CoreData:
				data.Gems = *obj.Gems
				gameObject.ObjectData = data
			case game.DepositData:
				data.Gems = *obj.Gems
				gameObject.ObjectData = data
			}
		}
		if obj.SpawnCooldown != nil {
			switch data := gameObject.ObjectData.(type) {
			case game.CoreData:
				data.SpawnCooldown = *obj.SpawnCooldown
				gameObject.ObjectData = data
			}
		}
	}

	// Decrement cooldowns for alive units
	for i := range g.Objects {
		if !g.Objects[i].IsAlive() {
			continue
		}
		switch data := g.Objects[i].ObjectData.(type) {
		case game.UnitData:
			if data.ActionCooldown != nil && *data.ActionCooldown > 0 {
				*data.ActionCooldown--
				g.Objects[i].ObjectData = data
			}
		}
	}

	for _, action := range tick.Errors {
		fmt.Printf("Error: %s\n", action)
	}
}

func initializeObjectData(obj incomingObject) game.ObjectData {
	if obj.Type == nil {
		return nil
	}
	switch *obj.Type {
	case game.ObjectUnit:
		return game.UnitData{
			UnitType: game.UnitWarrior,
			Gems:     obj.Gems,
		}
	case game.ObjectCore:
		d := game.CoreData{}
		if obj.TeamId != nil {
			d.TeamId = *obj.TeamId
		}
		if obj.Gems != nil {
			d.Gems = *obj.Gems
		}
		if obj.SpawnCooldown != nil {
			d.SpawnCooldown = *obj.SpawnCooldown
		}
		return d
	case game.ObjectDeposit:
		d := game.DepositData{}
		if obj.Gems != nil {
			d.Gems = *obj.Gems
		}
		return d
	case game.ObjectBomb:
		return game.BombData{}
	default:
		return nil
	}
}
