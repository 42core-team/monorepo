package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/shared"
)

type incomingObject struct {
	Id            *uint              `json:"id,omitempty"`
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

func (tick *GameTick) UpdateGame(game *shared.Game) {
	game.ElapsedTicks = tick.Tick
	fmt.Println("Updating unit cooldowns")
	for i := range game.Objects {
		switch data := game.Objects[i].ObjectData.(type) {
		case shared.UnitData:
			{
				if *data.ActionCooldown > 0 {
					fmt.Printf("Unit %d is on cooldown for %d more ticks\n", game.Objects[i].Id, data.ActionCooldown)
					*data.ActionCooldown--
					game.Objects[i].ObjectData = data
				}
			}
		default:
			continue
		}
	}

	// Update the game state based on the tick data
	for _, obj := range tick.Objects {
		if obj.Id == nil {
			fmt.Printf("Received object with nil ID: %v\n", obj)
			continue
		}
		gameObject, err := game.GetObjectById(*obj.Id)
		if err != nil {
			// Object doesn't exist, create a new one
			newObject := shared.NewObject(obj.Type, obj.Id, obj.X, obj.Y, obj.Hp, obj.TeamId, nil)
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

	for _, action := range tick.Errors {
		fmt.Printf("Error: %s\n", action)
	}
	// TODO: implement action handling when everything else is working
	//for _, action := range tick.Actions {
	//	fmt.Printf("Action: %s\n", action.GetType())
	//}
}
