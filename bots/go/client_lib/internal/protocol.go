package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/shared"
)

type incomingObject struct {
	Id            uint               `json:"id"`
	Type          *shared.ObjectType `json:"type,omitempty"`
	X             *uint              `json:"x,omitempty"`
	Y             *uint              `json:"y,omitempty"`
	Hp            *uint              `json:"hp,omitempty"`
	TeamId        *uint              `json:"teamId,omitempty"`
	Gems          *uint              `json:"gems,omitempty"`
	SpawnCooldown *uint              `json:"SpawnCooldown,omitempty"`
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
}

func NewGameTick(tickData []byte) (*GameTick, error) {

	tick := &GameTick{}

	if err := json.Unmarshal(tickData, tick); err != nil {
		return nil, fmt.Errorf("error unmarshalling game tick data: %v", err)
	}

	return tick, nil
}

func (tick *GameTick) UpdateGame(game *shared.Game) {

	for _, obj := range game.Objects {
		switch data := obj.ObjectData.(type) {
		case shared.UnitData:
			{
				if *data.ActionCooldown > 0 {
					fmt.Printf("Unit %d is on cooldown for %d more ticks\n", obj.Id, data.ActionCooldown)
					*data.ActionCooldown--
					obj.ObjectData = data
				}
			}
		default:
			continue
		}
	}

	// Update the game state based on the tick data
	for _, obj := range tick.Objects {
		gameObject, err := game.GetObjectById(obj.Id)
		if err != nil {
			// Object doesn't exist, create a new one
			newObject := shared.NewObject(*obj.Type, obj.Id, shared.Position{X: *obj.X, Y: *obj.Y}, int32(*obj.Hp), nil)
			game.Objects = append(game.Objects, *newObject)
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
			case shared.UnitData:
				data.Gems = obj.Gems
				gameObject.ObjectData = data
			case shared.CoreData:
				data.Gems = *obj.Gems
				gameObject.ObjectData = data
			case shared.DepositData:
				data.Gems = *obj.Gems
				gameObject.ObjectData = data
			default:
				continue
			}
		}
		if obj.SpawnCooldown != nil {
			switch data := gameObject.ObjectData.(type) {
			case shared.CoreData:
				data.SpawnCooldown = *obj.SpawnCooldown
				gameObject.ObjectData = data
			default:
				continue
			}
		}

	}

	// TODO: implement action handling when everything else is working
	//for _, action := range tick.Actions {
	//	fmt.Printf("Action: %s\n", action.GetType())
	//}
}
