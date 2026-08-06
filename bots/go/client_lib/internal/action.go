package internal

import (
	"encoding/json"

	"github.com/42core-team/go-client-lib/game"
)

type ActionType int

const (
	ActionCreate ActionType = iota
	ActionMove
	ActionAttack
	ActionTransfer
)

func (a ActionType) String() string {
	switch a {
	case ActionCreate:
		return "create"
	case ActionMove:
		return "move"
	case ActionAttack:
		return "attack"
	case ActionTransfer:
		return "transfer_gems"
	default:
		return "unknown"
	}
}

type Action struct {
	Type       ActionType
	Name       *string
	Components []string
	UnitID     uint
	TargetID   uint
	TargetPos  game.Position
	Amount     uint
}

func (a Action) MarshalJSON() ([]byte, error) {
	switch a.Type {
	case ActionCreate:
		components := a.Components
		if components == nil {
			components = []string{}
		}
		return json.Marshal(struct {
			Type       string   `json:"type"`
			Name       *string  `json:"name"`
			Components []string `json:"components"`
		}{a.Type.String(), a.Name, components})
	case ActionMove:
		return json.Marshal(struct {
			Type   string `json:"type"`
			UnitID uint   `json:"unit_id"`
			X      uint   `json:"x"`
			Y      uint   `json:"y"`
		}{a.Type.String(), a.UnitID, a.TargetPos.X, a.TargetPos.Y})
	case ActionAttack:
		return json.Marshal(struct {
			Type     string `json:"type"`
			UnitID   uint   `json:"unit_id"`
			TargetID uint   `json:"target_id"`
		}{a.Type.String(), a.UnitID, a.TargetID})
	case ActionTransfer:
		return json.Marshal(struct {
			Type     string `json:"type"`
			SourceID uint   `json:"source_id"`
			Amount   uint   `json:"amount"`
			X        uint   `json:"x"`
			Y        uint   `json:"y"`
		}{a.Type.String(), a.UnitID, a.Amount, a.TargetPos.X, a.TargetPos.Y})
	default:
		return json.Marshal(nil)
	}
}

type ActionQueue struct {
	actions  []Action
	capacity int
}

func NewActionQueue(capacity int) *ActionQueue {
	return &ActionQueue{actions: make([]Action, 0, capacity), capacity: capacity}
}
func (q *ActionQueue) Add(action Action) { q.actions = append(q.actions, action) }
func (q *ActionQueue) Drain() []Action {
	actions := q.actions
	q.actions = make([]Action, 0, q.capacity)
	return actions
}
func (q *ActionQueue) Reset() { q.actions = make([]Action, 0, q.capacity) }
