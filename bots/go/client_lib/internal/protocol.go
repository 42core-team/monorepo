package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

type incomingObject struct {
	ID            *uint            `json:"id,omitempty"`
	Type          *game.ObjectType `json:"type,omitempty"`
	X             *uint            `json:"x,omitempty"`
	Y             *uint            `json:"y,omitempty"`
	Hp            *uint            `json:"hp,omitempty"`
	TeamID        *uint            `json:"teamId,omitempty"`
	Gems          *uint            `json:"gems,omitempty"`
	SpawnCooldown *uint            `json:"SpawnCooldown,omitempty"`
}

type incomingAction struct {
	Type     *string `json:"type,omitempty"`
	UnitID   *uint   `json:"unit_id,omitempty"`
	X        *uint   `json:"x,omitempty"`
	Y        *uint   `json:"y,omitempty"`
	TargetID *uint   `json:"target_id,omitempty"`
	UnitType *uint   `json:"unit_type,omitempty"`
}

type gameTick struct {
	Objects []incomingObject `json:"objects"`
	Actions []incomingAction `json:"actions"`
	Errors  []string         `json:"errors"`
	Tick    uint             `json:"tick"`
}

func parseGameTick(tickData string) (*gameTick, error) {
	tick := &gameTick{}
	if err := json.Unmarshal([]byte(tickData), tick); err != nil {
		return nil, fmt.Errorf("error unmarshalling game tick: %v", err)
	}
	return tick, nil
}

func (tick *gameTick) applyTo(g *game.Game) {
	g.ElapsedTicks = tick.Tick

	for _, obj := range tick.Objects {
		if obj.ID == nil {
			continue
		}
		gameObject, err := g.ObjectByID(*obj.ID)
		if err != nil {
			newObj := game.Object{ID: *obj.ID}
			if obj.Type != nil {
				newObj.Type = *obj.Type
			}
			if obj.X != nil && obj.Y != nil {
				newObj.Pos = game.Position{X: *obj.X, Y: *obj.Y}
			}
			if obj.Hp != nil {
				newObj.Hp = int32(*obj.Hp)
			}
			if obj.TeamID != nil {
				newObj.TeamID = *obj.TeamID
			}
			newObj.ObjectData = initObjectData(obj)
			g.Objects = append(g.Objects, newObj)
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
		if obj.TeamID != nil {
			gameObject.TeamID = *obj.TeamID
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
			if data, ok := gameObject.ObjectData.(game.CoreData); ok {
				data.SpawnCooldown = *obj.SpawnCooldown
				gameObject.ObjectData = data
			}
		}
	}

	// Decrement cooldowns for alive units.
	for i := range g.Objects {
		if !g.Objects[i].IsAlive() {
			continue
		}
		if data, ok := g.Objects[i].ObjectData.(game.UnitData); ok {
			if data.ActionCooldown != nil && *data.ActionCooldown > 0 {
				*data.ActionCooldown--
				g.Objects[i].ObjectData = data
			}
		}
	}

	for _, errMsg := range tick.Errors {
		fmt.Printf("Error: %s\n", errMsg)
	}
}

func initObjectData(obj incomingObject) game.ObjectData {
	if obj.Type == nil {
		return nil
	}
	switch *obj.Type {
	case game.ObjectUnit:
		return game.UnitData{UnitType: game.UnitWarrior, Gems: obj.Gems}
	case game.ObjectCore:
		d := game.CoreData{}
		if obj.TeamID != nil {
			d.TeamID = *obj.TeamID
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
