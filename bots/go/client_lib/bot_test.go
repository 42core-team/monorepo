package core

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"testing"
	"time"

	"github.com/42core-team/go-client-lib/game"
)

type serverResult struct {
	packets []map[string]json.RawMessage
	err     error
}

func mockGameServer(t *testing.T, ticks ...string) (BotConfig, <-chan serverResult) {
	t.Helper()
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = listener.Close() })
	done := make(chan serverResult, 1)
	go func() {
		result := serverResult{}
		defer func() { done <- result }()
		peer, err := listener.Accept()
		if err != nil {
			result.err = err
			return
		}
		defer peer.Close()
		_ = peer.SetDeadline(time.Now().Add(5 * time.Second))
		reader := bufio.NewReader(peer)
		if _, result.err = reader.ReadString('\n'); result.err != nil {
			return
		}
		if _, result.err = fmt.Fprintln(peer, `{"gridSize":4}`); result.err != nil {
			return
		}
		for _, tick := range ticks {
			var line string
			if line, result.err = reader.ReadString('\n'); result.err != nil {
				return
			}
			var packet map[string]json.RawMessage
			if result.err = json.Unmarshal([]byte(line), &packet); result.err != nil {
				return
			}
			result.packets = append(result.packets, packet)
			if _, result.err = fmt.Fprintln(peer, tick); result.err != nil {
				return
			}
		}
	}()
	return BotConfig{TeamID: 1, TeamName: "Test", ServerAddr: listener.Addr().String()}, done
}

func TestStartGameGlobalAPIAndRestart(t *testing.T) {
	previousGame := Game
	t.Cleanup(func() { Game = previousGame })
	for run := 0; run < 2; run++ {
		config, done := mockGameServer(t,
			`{"tick":1,"objects":[{"id":1,"type":0,"hp":50,"teamId":1},{"id":2,"type":0,"hp":50,"teamId":2},{"id":7,"type":1,"x":1,"y":1,"hp":5,"teamId":1,"name":"Runner"}]}`,
			`{"tick":2,"objects":[{"id":2,"hp":0}]}`)
		callbacks := 0
		err := StartGame(config, func(tick uint) {
			callbacks++
			if tick != uint(callbacks) || Game.ElapsedTicks != tick || Game.MyTeamID != 1 || Game.Config.GridSize != 4 {
				t.Errorf("callback saw stale state: tick %d, %#v", tick, Game)
			}
			if err := StartGame(config, func(uint) {}); !errors.Is(err, ErrGameAlreadyRunning) {
				t.Errorf("nested start: %v", err)
			}
			concurrent := make(chan error, 1)
			go func() { concurrent <- StartGame(config, func(uint) {}) }()
			if err := <-concurrent; !errors.Is(err, ErrGameAlreadyRunning) {
				t.Errorf("concurrent start: %v", err)
			}
			unit, err := ObjectByID(7)
			if err != nil {
				t.Error(err)
				return
			}
			if ObjectAtPos(unit.Pos) != unit || UnitsByNameCount("Runner") != 1 || len(UnitsByName("Runner")) != 1 || ObjectsFilterCount(nil) != 3 {
				t.Error("global getters did not read current state")
			}
			if len(ObjectsFilter(func(o *game.Object) bool { return o.Type == game.ObjectUnit })) != 1 || NearestObject(unit.Pos, func(o *game.Object) bool { return o.Type == game.ObjectUnit }) != unit {
				t.Error("filter/nearest getters did not read current state")
			}
			if !IsPosValid(unit.Pos) || Distance(unit.Pos, game.Position{}) != 2 {
				t.Error("position helpers failed")
			}
			if err := Move(unit, game.Position{X: 2, Y: 1}); err != nil {
				t.Error(err)
			}
			AddObjectInfo(unit, "run %d", run)
			AddObjectPathStep(unit, game.Position{X: 2, Y: 1})
		})
		if err != nil {
			t.Fatal(err)
		}
		result := <-done
		if result.err != nil || callbacks != 2 || len(result.packets) != 2 {
			t.Fatalf("callbacks=%d, result=%#v", callbacks, result)
		}
		if string(result.packets[0]["actions"]) != "[]" || string(result.packets[0]["debug_data"]) != "[]" {
			t.Fatal("new game inherited old queues")
		}
		var actions []map[string]any
		var debug []map[string]any
		if err := json.Unmarshal(result.packets[1]["actions"], &actions); err != nil {
			t.Fatal(err)
		}
		if err := json.Unmarshal(result.packets[1]["debug_data"], &debug); err != nil {
			t.Fatal(err)
		}
		if len(actions) != 1 || actions[0]["type"] != "move" || len(debug) != 1 {
			t.Fatalf("actions/debug not sent: %#v", result.packets[1])
		}
		if running.Load() || connection.Load() != nil || !errors.Is(CreateUnit("test"), ErrClientUnavailable) {
			t.Fatal("runtime not cleaned up")
		}
	}
}

func TestStartGameProtocolFailureCleansUp(t *testing.T) {
	previousGame := Game
	t.Cleanup(func() { Game = previousGame })
	config, done := mockGameServer(t, "{invalid}")
	err := StartGame(config, func(uint) { t.Error("callback called for invalid state") })
	var syntax *json.SyntaxError
	if !errors.As(err, &syntax) {
		t.Fatalf("expected wrapped syntax error, got %v", err)
	}
	if result := <-done; result.err != nil {
		t.Fatal(result.err)
	}
	if running.Load() || connection.Load() != nil {
		t.Fatal("runtime not cleaned up after failure")
	}
}

func TestStartGameRejectsNilCallback(t *testing.T) {
	if err := StartGame(BotConfig{}, nil); err == nil || running.Load() {
		t.Fatalf("nil callback: %v", err)
	}
}

func TestCloseFromAnotherGoroutine(t *testing.T) {
	previousGame := Game
	t.Cleanup(func() { Game = previousGame })
	config, done := mockGameServer(t,
		`{"tick":1,"objects":[{"id":1,"type":0,"hp":50,"teamId":1},{"id":2,"type":0,"hp":50,"teamId":2}]}`)
	err := StartGame(config, func(uint) {
		closed := make(chan error, 1)
		go func() { closed <- Close() }()
		if err := <-closed; err != nil {
			t.Error(err)
		}
		if err := Close(); err != nil {
			t.Errorf("repeated close: %v", err)
		}
		if err := CreateUnit("test"); !errors.Is(err, ErrClientUnavailable) {
			t.Errorf("action after close: %v", err)
		}
	})
	if err == nil {
		t.Fatal("closing an unfinished game should return a transport error")
	}
	if result := <-done; result.err != nil {
		t.Fatal(result.err)
	}
	if running.Load() || connection.Load() != nil {
		t.Fatal("runtime not cleaned up after close")
	}
}

func TestDebugWithoutActiveGame(t *testing.T) {
	AddObjectInfo(&game.Object{ID: 1}, "ignored")
	AddObjectPathStep(&game.Object{ID: 1}, game.Position{})
	if err := Close(); !errors.Is(err, ErrClientUnavailable) {
		t.Fatalf("close without game: %v", err)
	}
}
