package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

type incomingObject struct {
	ID             *uint            `json:"id,omitempty"`
	Type           *game.ObjectType `json:"type,omitempty"`
	X              *uint            `json:"x,omitempty"`
	Y              *uint            `json:"y,omitempty"`
	Hp             *uint            `json:"hp,omitempty"`
	TeamID         *uint            `json:"teamId,omitempty"`
	Gems           *uint            `json:"gems,omitempty"`
	UnitType       *game.UnitType   `json:"unit_type,omitempty"`
	ActionCooldown *uint            `json:"ActionCooldown,omitempty"`
	SpawnCooldown  *uint            `json:"SpawnCooldown,omitempty"`
	Countdown      *uint            `json:"countdown,omitempty"`
	State          *string          `json:"state,omitempty"`
}

type gameTick struct {
	Objects []incomingObject `json:"objects"`
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

		if obj.State != nil && *obj.State == "dead" {
			g.Objects = removeObject(g.Objects, *obj.ID)
			continue
		}

		existingObj, _ := g.ObjectByID(*obj.ID)
		if existingObj == nil {
			newObj := game.Object{ID: *obj.ID}
			if obj.Type != nil {
				newObj.Type = *obj.Type
			}
			if obj.X != nil && obj.Y != nil {
				newObj.Pos = game.Position{X: *obj.X, Y: *obj.Y}
			}
			if obj.Hp != nil {
				newObj.Hp = *obj.Hp
			}
			if obj.TeamID != nil {
				newObj.TeamID = *obj.TeamID
			}
			newObj.ObjectData = initObjectData(obj)
			g.Objects = append(g.Objects, &newObj)
			continue
		}

		if obj.X != nil {
			existingObj.Pos.X = *obj.X
		}
		if obj.Y != nil {
			existingObj.Pos.Y = *obj.Y
		}
		if obj.Hp != nil {
			existingObj.Hp = *obj.Hp
		}
		if obj.TeamID != nil {
			existingObj.TeamID = *obj.TeamID
		}
		updateObjectData(existingObj, obj)
	}

	for i := range g.Objects {
		if !g.Objects[i].IsAlive() {
			continue
		}
		if data := g.Objects[i].GetUnitData(); data != nil {
			if data.ActionCooldown != nil && *data.ActionCooldown > 0 {
				*data.ActionCooldown--
				g.Objects[i].ObjectData = *data
			}
		}
		if data := g.Objects[i].GetCoreData(); data != nil {
			if data.SpawnCooldown > 0 {
				data.SpawnCooldown--
				g.Objects[i].ObjectData = *data
			}
		}
	}

	for _, errMsg := range tick.Errors {
		fmt.Printf("\033[31m%s\033[0m\n", errMsg)
	}
}

func removeObject(objects []*game.Object, id uint) []*game.Object {
	for i, obj := range objects {
		if obj.ID == id {
			return append(objects[:i], objects[i+1:]...)
		}
	}
	return objects
}

func initObjectData(obj incomingObject) game.ObjectData {
	if obj.Type == nil {
		return nil
	}
	switch *obj.Type {
	case game.ObjectUnit:
		data := game.UnitData{}
		if obj.UnitType != nil {
			data.UnitType = *obj.UnitType
		}
		if obj.TeamID != nil {
			data.TeamID = *obj.TeamID
		}
		if obj.Gems != nil {
			data.Gems = obj.Gems
		}
		if obj.ActionCooldown != nil {
			data.ActionCooldown = obj.ActionCooldown
		}
		return data
	case game.ObjectCore:
		data := game.CoreData{}
		if obj.TeamID != nil {
			data.TeamID = *obj.TeamID
		}
		if obj.Gems != nil {
			data.Gems = *obj.Gems
		}
		if obj.SpawnCooldown != nil {
			data.SpawnCooldown = *obj.SpawnCooldown
		}
		return data
	case game.ObjectDeposit, game.ObjectGemPile:
		data := game.DepositData{}
		if obj.Gems != nil {
			data.Gems = *obj.Gems
		}
		return data
	case game.ObjectBomb:
		data := game.BombData{}
		if obj.Countdown != nil {
			data.Countdown = *obj.Countdown
		}
		return data
	default:
		return nil
	}
}

func updateObjectData(obj *game.Object, incoming incomingObject) {
	switch obj.Type {
	case game.ObjectUnit:
		data := obj.GetUnitData()
		if data == nil {
			data = &game.UnitData{}
		}
		if incoming.UnitType != nil {
			data.UnitType = *incoming.UnitType
		}
		if incoming.TeamID != nil {
			data.TeamID = *incoming.TeamID
		}
		if incoming.Gems != nil {
			data.Gems = incoming.Gems
		}
		if incoming.ActionCooldown != nil {
			data.ActionCooldown = incoming.ActionCooldown
		}
		obj.ObjectData = *data
	case game.ObjectCore:
		data := obj.GetCoreData()
		if data == nil {
			data = &game.CoreData{}
		}
		if incoming.TeamID != nil {
			data.TeamID = *incoming.TeamID
		}
		if incoming.Gems != nil {
			data.Gems = *incoming.Gems
		}
		if incoming.SpawnCooldown != nil {
			data.SpawnCooldown = *incoming.SpawnCooldown
		}
		obj.ObjectData = *data
	case game.ObjectDeposit, game.ObjectGemPile:
		data := obj.GetDepositData()
		if data == nil {
			data = &game.DepositData{}
		}
		if incoming.Gems != nil {
			data.Gems = *incoming.Gems
		}
		obj.ObjectData = *data
	case game.ObjectBomb:
		data := obj.GetBombData()
		if data == nil {
			data = &game.BombData{}
		}
		if incoming.Countdown != nil {
			data.Countdown = *incoming.Countdown
		}
		obj.ObjectData = *data
	}
}
