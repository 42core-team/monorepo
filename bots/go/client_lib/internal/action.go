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
	ActionBuild
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
	case ActionBuild:
		return "build"
	default:
		return "unknown"
	}
}

type Action struct {
	Type      ActionType
	UnitType  game.UnitType
	UnitID    uint
	TargetID  uint
	TargetPos game.Position
	Amount    uint
}

func (a Action) MarshalJSON() ([]byte, error) {
	switch a.Type {
	case ActionCreate:
		return json.Marshal(struct {
			Type     string `json:"type"`
			UnitType int    `json:"unit_type"`
		}{
			Type:     a.Type.String(),
			UnitType: int(a.UnitType),
		})

	case ActionMove:
		return json.Marshal(struct {
			Type   string `json:"type"`
			UnitID uint   `json:"unit_id"`
			X      uint   `json:"x"`
			Y      uint   `json:"y"`
		}{
			Type:   a.Type.String(),
			UnitID: a.UnitID,
			X:      a.TargetPos.X,
			Y:      a.TargetPos.Y,
		})

	case ActionAttack:
		return json.Marshal(struct {
			Type     string `json:"type"`
			UnitID   uint   `json:"unit_id"`
			TargetID uint   `json:"target_id"`
		}{
			Type:     a.Type.String(),
			UnitID:   a.UnitID,
			TargetID: a.TargetID,
		})

	case ActionTransfer:
		return json.Marshal(struct {
			Type     string `json:"type"`
			SourceID uint   `json:"source_id"`
			Amount   uint   `json:"amount"`
			X        uint   `json:"x"`
			Y        uint   `json:"y"`
		}{
			Type:     a.Type.String(),
			SourceID: a.UnitID,
			Amount:   a.Amount,
			X:        a.TargetPos.X,
			Y:        a.TargetPos.Y,
		})

	case ActionBuild:
		return json.Marshal(struct {
			Type   string `json:"type"`
			UnitID uint   `json:"unit_id"`
			X      uint   `json:"x"`
			Y      uint   `json:"y"`
		}{
			Type:   a.Type.String(),
			UnitID: a.UnitID,
			X:      a.TargetPos.X,
			Y:      a.TargetPos.Y,
		})
	}

	return nil, nil
}

type ActionQueue struct {
	actions  []Action
	capacity int
}

func NewActionQueue(capacity int) *ActionQueue {
	return &ActionQueue{
		actions:  make([]Action, 0, capacity),
		capacity: capacity,
	}
}

func (q *ActionQueue) Add(action Action) {
	q.actions = append(q.actions, action)
}

func (q *ActionQueue) Drain() []Action {
	actions := q.actions
	q.actions = make([]Action, 0, q.capacity)
	return actions
}

func (q *ActionQueue) Reset() {
	q.actions = make([]Action, 0, q.capacity)
}
