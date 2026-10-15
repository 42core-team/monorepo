package core

import (
	"errors"
	"net"
	"testing"

	"github.com/42core-team/go-client-lib/game"
	"github.com/42core-team/go-client-lib/internal"
)

func connectedTestGame(t *testing.T) *internal.Connection {
	t.Helper()
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = listener.Close() })
	conn, err := internal.NewConnection(listener.Addr().String(), 1, false)
	if err != nil {
		t.Fatal(err)
	}
	previousGame := Game
	Game = conn.Game()
	connection.Store(conn)
	t.Cleanup(func() {
		_ = conn.Close()
		connection.Store(nil)
		Game = previousGame
	})
	peer, err := listener.Accept()
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = peer.Close() })
	Game.Config.GridSize = 4
	return conn
}

func TestActionInputErrorsDoNotQueue(t *testing.T) {
	conn := connectedTestGame(t)
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{})
	wall := &game.Object{ID: 2, Type: game.ObjectWall}
	outside := game.Position{X: 4}
	cases := []struct {
		name string
		call func() error
		want error
	}{
		{"nil mover", func() error { return Move(nil, game.Position{}) }, game.ErrNilObject},
		{"wrong mover", func() error { return Move(wall, game.Position{}) }, game.ErrWrongObjectType},
		{"invalid move position", func() error { return Move(unit, outside) }, ErrInvalidPosition},
		{"nil attacker", func() error { return Attack(nil, wall) }, game.ErrNilObject},
		{"wrong attacker", func() error { return Attack(wall, unit) }, game.ErrWrongObjectType},
		{"nil attack target", func() error { return Attack(unit, nil) }, game.ErrNilObject},
		{"nil source", func() error { return TransferGems(nil, game.Position{}, 1) }, game.ErrNilObject},
		{"wrong source", func() error { return TransferGems(wall, game.Position{}, 1) }, game.ErrWrongObjectType},
		{"invalid transfer position", func() error { return TransferGems(unit, outside, 1) }, ErrInvalidPosition},
		{"nil traveler", func() error { return Travel(nil, game.Position{}, nil) }, game.ErrNilObject},
		{"wrong traveler", func() error { return Travel(wall, game.Position{}, nil) }, game.ErrWrongObjectType},
		{"missing travel payload", func() error { return Travel(&game.Object{Type: game.ObjectUnit}, game.Position{}, nil) }, game.ErrInvalidObjectData},
		{"invalid travel target", func() error { return Travel(unit, outside, nil) }, ErrInvalidPosition},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if err := tc.call(); !errors.Is(err, tc.want) {
				t.Fatalf("got %v, want %v", err, tc.want)
			}
			if actions := conn.ActionQueue().Drain(); len(actions) != 0 {
				t.Fatalf("invalid action queued: %#v", actions)
			}
		})
	}
}

func TestActionsLeaveGameplayValidationToServer(t *testing.T) {
	conn := connectedTestGame(t)
	// Foreign, cooling down, empty-handed, and far from the target: these
	// conditions must not prevent local queuing.
	unit := testUnit(7, 0, 0, 2, game.UnitProperties{})
	data, err := unit.UnitData()
	if err != nil {
		t.Fatal(err)
	}
	data.ActionCooldown = 10
	target := &game.Object{ID: 8, Type: game.ObjectCore, Pos: game.Position{X: 3, Y: 3}}
	Game.Objects = []*game.Object{unit, target}
	for _, err := range []error{
		Move(unit, target.Pos),
		Attack(unit, target),
		TransferGems(unit, target.Pos, 100),
		TransferGems(target, unit.Pos, 100),
		CreateUnit("Invalid design", "unknown-component"),
		Travel(unit, game.Position{X: 1}, nil),
	} {
		if err != nil {
			t.Fatal(err)
		}
	}
	if actions := conn.ActionQueue().Drain(); len(actions) != 6 {
		t.Fatalf("expected six queued actions, got %#v", actions)
	}
}

func TestActionsRejectUnavailableClient(t *testing.T) {
	closed := connectedTestGame(t)
	if err := closed.Close(); err != nil {
		t.Fatal(err)
	}
	for _, conn := range []*internal.Connection{nil, closed} {
		connection.Store(conn)
		calls := []func() error{
			func() error { return CreateUnit("test") },
			func() error { return Move(nil, game.Position{}) },
			func() error { return Attack(nil, nil) },
			func() error { return TransferGems(nil, game.Position{}, 1) },
			func() error { return Travel(nil, game.Position{}, nil) },
		}
		for _, call := range calls {
			if err := call(); !errors.Is(err, ErrClientUnavailable) {
				t.Fatalf("got %v, want unavailable client", err)
			}
		}
	}
}
