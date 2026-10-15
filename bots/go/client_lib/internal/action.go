package internal

import (
	"encoding/json"

	"github.com/42core-team/go-client-lib/game"
)

// An ActionType identifies the kind of an [Action].
type ActionType int

const (
	ActionCreate   ActionType = iota // create a unit
	ActionMove                       // move a unit
	ActionAttack                     // attack an object
	ActionTransfer                   // transfer gems
)

// String returns the name of a as used in the protocol.
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

// An Action is a single bot command sent to the server. Which fields are
// used depends on Type; see [Action.MarshalJSON].
type Action struct {
	Type       ActionType
	Name       *string       // create: unit name, or nil for a generated name
	Components []string      // create: component IDs
	UnitID     uint          // move, attack: acting unit; transfer: source object
	TargetID   uint          // attack: target object
	TargetPos  game.Position // move: destination; transfer: target tile
	Amount     uint          // transfer: number of gems
}

// MarshalJSON implements [json.Marshaler]. It encodes only the fields used
// by a.Type, and encodes an action of unknown type as null.
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

// An ActionQueue collects the actions queued during one tick.
type ActionQueue struct {
	actions  []Action
	capacity int
}

// NewActionQueue returns an empty queue that preallocates room for capacity
// actions per tick.
func NewActionQueue(capacity int) *ActionQueue {
	return &ActionQueue{actions: make([]Action, 0, capacity), capacity: capacity}
}

// Add appends action to the queue.
func (q *ActionQueue) Add(action Action) { q.actions = append(q.actions, action) }

// Drain returns the queued actions in order and empties the queue.
func (q *ActionQueue) Drain() []Action {
	actions := q.actions
	q.actions = make([]Action, 0, q.capacity)
	return actions
}

// Reset discards all queued actions.
func (q *ActionQueue) Reset() { q.actions = make([]Action, 0, q.capacity) }
