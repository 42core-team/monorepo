package internal

import (
	"bufio"
	"encoding/json"
	"net"
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestConnectionProtocolRoundTrip(t *testing.T) {
	client, server := net.Pipe()
	connection := &Connection{
		socket: client, reader: bufio.NewReader(client), game: &game.Game{MyTeamID: 3},
		actionQueue: NewActionQueue(4), debugData: NewDebugData(4),
	}
	connection.SetTickCallback(func(g *game.Game) {
		connection.actionQueue.Add(Action{Type: ActionMove, UnitID: 7, TargetPos: game.Position{X: 2, Y: 1}})
	})

	type result struct {
		packet map[string]any
		err    error
	}
	serverResult := make(chan result, 1)
	go func() {
		defer server.Close()
		scanner := bufio.NewScanner(server)
		if !scanner.Scan() {
			serverResult <- result{err: scanner.Err()}
			return
		}
		var login map[string]any
		if err := json.Unmarshal(scanner.Bytes(), &login); err != nil {
			serverResult <- result{err: err}
			return
		}
		if login["id"] != float64(3) || login["name"] != "Round Trip" || login["password"] != "42" {
			serverResult <- result{err: &protocolTestError{"unexpected login"}}
			return
		}
		_, _ = server.Write([]byte(`{"gridSize":4,"components":{"components":[]}}` + "\n"))
		if !scanner.Scan() {
			serverResult <- result{err: scanner.Err()}
			return
		}
		var first map[string]any
		if err := json.Unmarshal(scanner.Bytes(), &first); err != nil {
			serverResult <- result{err: err}
			return
		}
		debugData, ok := first["debug_data"].([]any)
		if !ok || len(debugData) != 0 {
			serverResult <- result{err: &protocolTestError{"debug_data must be an empty array"}}
			return
		}
		_, _ = server.Write([]byte(`{"tick":1,"objects":[{"id":1,"type":0,"x":0,"y":0,"hp":50,"teamId":3},{"id":2,"type":0,"x":3,"y":3,"hp":50,"teamId":4},{"id":7,"type":1,"x":1,"y":1,"hp":5,"teamId":3,"ActionCooldown":0,"components":["mobility"],"properties":{},"name":"Runner"}],"errors":[]}` + "\n"))
		if !scanner.Scan() {
			serverResult <- result{err: scanner.Err()}
			return
		}
		var second map[string]any
		if err := json.Unmarshal(scanner.Bytes(), &second); err != nil {
			serverResult <- result{err: err}
			return
		}
		serverResult <- result{packet: second}
	}()

	if err := connection.Start(3, "Round Trip"); err != nil {
		t.Fatal(err)
	}
	got := <-serverResult
	if got.err != nil {
		t.Fatal(got.err)
	}
	actions, ok := got.packet["actions"].([]any)
	if !ok || len(actions) != 1 {
		t.Fatalf("unexpected packet: %#v", got.packet)
	}
	action := actions[0].(map[string]any)
	if action["type"] != "move" || action["unit_id"] != float64(7) {
		t.Fatalf("unexpected action: %#v", action)
	}
	if connection.game.Config.GridSize != 4 || connection.game.ElapsedTicks != 1 {
		t.Fatalf("state not updated: %#v", connection.game)
	}
}

type protocolTestError struct{ message string }

func (e *protocolTestError) Error() string { return e.message }
