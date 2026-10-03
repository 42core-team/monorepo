package internal

import (
	"encoding/json"
	"fmt"

	"github.com/42core-team/go-client-lib/game"
)

type incomingObject struct {
	ID             *uint                   `json:"id,omitempty"`
	Type           *game.ObjectType        `json:"type,omitempty"`
	X              *uint                   `json:"x,omitempty"`
	Y              *uint                   `json:"y,omitempty"`
	HP             *uint                   `json:"hp,omitempty"`
	TeamID         *uint                   `json:"teamId,omitempty"`
	Gems           *uint                   `json:"gems,omitempty"`
	ActionCooldown *int                    `json:"ActionCooldown,omitempty"`
	SpawnCooldown  *uint                   `json:"SpawnCooldown,omitempty"`
	Components     []string                `json:"components,omitempty"`
	Properties     *incomingUnitProperties `json:"properties,omitempty"`
	Name           *string                 `json:"name,omitempty"`
	State          *string                 `json:"state,omitempty"`
}

type incomingUnitProperties struct {
	Hp                     *uint `json:"hp,omitempty"`
	BaseActionCooldown     *int  `json:"baseActionCooldown,omitempty"`
	GemsPerCooldownStep    *uint `json:"gemsPerCooldownStep,omitempty"`
	MaxGems                *uint `json:"maxGems,omitempty"`
	DamageReductionPercent *int  `json:"damageReductionPercent,omitempty"`
	DamageCore             *uint `json:"damageCore,omitempty"`
	DamageUnit             *uint `json:"damageUnit,omitempty"`
	DamageObject           *uint `json:"damageObject,omitempty"`
	PostSpawnCoreCooldown  *uint `json:"postSpawnCoreCooldown,omitempty"`
}

type gameTick struct {
	Objects []incomingObject `json:"objects"`
	Errors  []string         `json:"errors"`
	Tick    uint             `json:"tick"`
}

func parseGameTick(tickData string) (*gameTick, error) {
	tick := &gameTick{}
	if err := json.Unmarshal([]byte(tickData), tick); err != nil {
		return nil, fmt.Errorf("error unmarshalling game tick: %w", err)
	}
	return tick, nil
}

func (tick *gameTick) applyTo(g *game.Game) {
	g.ElapsedTicks = tick.Tick
	for _, update := range tick.Objects {
		if update.ID == nil {
			continue
		}
		if update.State != nil && *update.State == "dead" {
			g.Objects = removeObject(g.Objects, *update.ID)
			continue
		}
		object, _ := g.ObjectByID(*update.ID)
		if object == nil {
			object = &game.Object{ID: *update.ID}
			g.Objects = append(g.Objects, object)
		}
		applyObjectUpdate(object, update)
	}
	for _, message := range tick.Errors {
		fmt.Printf("\033[31m%s\033[0m\n", message)
	}
}

func applyObjectUpdate(object *game.Object, update incomingObject) {
	if update.Type != nil {
		object.Type = *update.Type
	}
	if update.X != nil {
		object.Pos.X = *update.X
	}
	if update.Y != nil {
		object.Pos.Y = *update.Y
	}
	if update.HP != nil {
		object.Hp = *update.HP
	}
	if update.TeamID != nil {
		object.TeamID = *update.TeamID
	}
	switch object.Type {
	case game.ObjectUnit:
		data := object.GetUnitData()
		if data == nil {
			data = &game.UnitData{}
		}
		if update.TeamID != nil {
			data.TeamID = *update.TeamID
		}
		if update.Gems != nil {
			data.Gems = *update.Gems
		}
		if update.ActionCooldown != nil {
			data.ActionCooldown = *update.ActionCooldown
		}
		if update.Components != nil {
			data.Components = append(data.Components[:0], update.Components...)
		}
		if update.Properties != nil {
			applyUnitProperties(&data.Properties, update.Properties)
		}
		if update.Name != nil {
			data.Name = *update.Name
		}
		object.ObjectData = data
	case game.ObjectCore:
		data := object.GetCoreData()
		if data == nil {
			data = &game.CoreData{}
		}
		if update.TeamID != nil {
			data.TeamID = *update.TeamID
		}
		if update.Gems != nil {
			data.Gems = *update.Gems
		}
		if update.SpawnCooldown != nil {
			data.SpawnCooldown = *update.SpawnCooldown
		}
		object.ObjectData = data
	case game.ObjectDeposit, game.ObjectGemPile:
		data := object.GetDepositData()
		if data == nil {
			data = &game.DepositData{}
		}
		if update.Gems != nil {
			data.Gems = *update.Gems
		}
		object.ObjectData = data
	}
}

func applyUnitProperties(properties *game.UnitProperties, update *incomingUnitProperties) {
	if update.Hp != nil {
		properties.Hp = *update.Hp
	}
	if update.BaseActionCooldown != nil {
		properties.BaseActionCooldown = *update.BaseActionCooldown
	}
	if update.GemsPerCooldownStep != nil {
		properties.GemsPerCooldownStep = *update.GemsPerCooldownStep
	}
	if update.MaxGems != nil {
		properties.MaxGems = *update.MaxGems
	}
	if update.DamageReductionPercent != nil {
		properties.DamageReductionPercent = *update.DamageReductionPercent
	}
	if update.DamageCore != nil {
		properties.DamageCore = *update.DamageCore
	}
	if update.DamageUnit != nil {
		properties.DamageUnit = *update.DamageUnit
	}
	if update.DamageObject != nil {
		properties.DamageObject = *update.DamageObject
	}
	if update.PostSpawnCoreCooldown != nil {
		properties.PostSpawnCoreCooldown = *update.PostSpawnCoreCooldown
	}
}

func removeObject(objects []*game.Object, id uint) []*game.Object {
	for i, object := range objects {
		if object.ID == id {
			return append(objects[:i], objects[i+1:]...)
		}
	}
	return objects
}
