package internal

import (
	"encoding/json"
	"reflect"
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestActionJSONMatchesServerProtocol(t *testing.T) {
	name := "Scout"
	tests := []struct {
		name   string
		action Action
		want   map[string]any
	}{
		{"create", Action{Type: ActionCreate, Name: &name, Components: []string{"mobility", "vision"}}, map[string]any{"type": "create", "name": "Scout", "components": []any{"mobility", "vision"}}},
		{"create generated name", Action{Type: ActionCreate, Components: []string{"combat"}}, map[string]any{"type": "create", "name": nil, "components": []any{"combat"}}},
		{"create no components", Action{Type: ActionCreate}, map[string]any{"type": "create", "name": nil, "components": []any{}}},
		{"move", Action{Type: ActionMove, UnitID: 7, TargetPos: game.Position{X: 2, Y: 3}}, map[string]any{"type": "move", "unit_id": float64(7), "x": float64(2), "y": float64(3)}},
		{"attack", Action{Type: ActionAttack, UnitID: 7, TargetID: 9}, map[string]any{"type": "attack", "unit_id": float64(7), "target_id": float64(9)}},
		{"transfer", Action{Type: ActionTransfer, UnitID: 7, TargetPos: game.Position{X: 2, Y: 3}, Amount: 5}, map[string]any{"type": "transfer_gems", "source_id": float64(7), "x": float64(2), "y": float64(3), "amount": float64(5)}},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			encoded, err := json.Marshal(test.action)
			if err != nil {
				t.Fatal(err)
			}
			var got map[string]any
			if err := json.Unmarshal(encoded, &got); err != nil {
				t.Fatal(err)
			}
			if !reflect.DeepEqual(got, test.want) {
				t.Fatalf("got %#v, want %#v", got, test.want)
			}
		})
	}
}
