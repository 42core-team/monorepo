package internal

import (
	"bufio"
	"encoding/json"
	"errors"
	"io"
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

	if err := connection.Start(3, "Round Trip"); !errors.Is(err, io.EOF) && !errors.Is(err, io.ErrClosedPipe) {
		t.Fatalf("expected wrapped transport error after server disconnect, got %v", err)
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

func TestConnectionCompletionAndProtocolFailures(t *testing.T) {
	cases := []struct {
		name string
		tick string
		want error
	}{
		{"completed game", `{"tick":1,"objects":[{"id":1,"type":0,"hp":0,"teamId":3}],"errors":[]}` + "\n", nil},
		{"won game", `{"tick":1,"objects":[{"id":1,"type":0,"hp":50,"teamId":3}],"errors":[]}` + "\n", nil},
		{"malformed JSON", "{bad json}\n", &json.SyntaxError{}},
		{"truncated frame", `{"tick":1}`, io.EOF},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			client, server := net.Pipe()
			connection := &Connection{
				socket: client, reader: bufio.NewReader(client), game: &game.Game{MyTeamID: 3},
				actionQueue: NewActionQueue(4), debugData: NewDebugData(4),
			}
			defer connection.Close()
			done := make(chan struct{})
			go func() {
				defer close(done)
				defer server.Close()
				reader := bufio.NewReader(server)
				if _, err := reader.ReadString('\n'); err != nil {
					return
				}
				if _, err := server.Write([]byte("{\"gridSize\":4}\n")); err != nil {
					return
				}
				if _, err := reader.ReadString('\n'); err != nil {
					return
				}
				_, _ = server.Write([]byte(tc.tick))
			}()
			err := connection.Start(3, "Test")
			<-done
			if _, syntax := tc.want.(*json.SyntaxError); syntax {
				var target *json.SyntaxError
				if !errors.As(err, &target) {
					t.Fatalf("expected wrapped syntax error, got %v", err)
				}
			} else if !errors.Is(err, tc.want) {
				t.Fatalf("got %v, want %v", err, tc.want)
			}
			if !connection.IsClosed() {
				t.Fatal("connection was not closed")
			}
		})
	}
}
