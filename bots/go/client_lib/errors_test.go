package coregame

import (
	"errors"
	"net"
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func connectedTestBot(t *testing.T) *Bot {
	t.Helper()
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = listener.Close() })
	b, err := NewBot(BotConfig{TeamID: 1, ServerAddr: listener.Addr().String()})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = b.Close() })
	peer, err := listener.Accept()
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = peer.Close() })
	b.game.Config.GridSize = 4
	return b
}

func TestActionInputErrorsDoNotQueue(t *testing.T) {
	b := connectedTestBot(t)
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{})
	wall := &game.Object{ID: 2, Type: game.ObjectWall}
	outside := game.Position{X: 4}
	cases := []struct {
		name string
		call func() error
		want error
	}{
		{"nil mover", func() error { return b.Move(nil, game.Position{}) }, game.ErrNilObject},
		{"wrong mover", func() error { return b.Move(wall, game.Position{}) }, game.ErrWrongObjectType},
		{"invalid move position", func() error { return b.Move(unit, outside) }, ErrInvalidPosition},
		{"nil attacker", func() error { return b.Attack(nil, wall) }, game.ErrNilObject},
		{"wrong attacker", func() error { return b.Attack(wall, unit) }, game.ErrWrongObjectType},
		{"nil attack target", func() error { return b.Attack(unit, nil) }, game.ErrNilObject},
		{"nil source", func() error { return b.TransferGems(nil, game.Position{}, 1) }, game.ErrNilObject},
		{"wrong source", func() error { return b.TransferGems(wall, game.Position{}, 1) }, game.ErrWrongObjectType},
		{"invalid transfer position", func() error { return b.TransferGems(unit, outside, 1) }, ErrInvalidPosition},
		{"nil traveler", func() error { return b.Travel(nil, game.Position{}, nil) }, game.ErrNilObject},
		{"wrong traveler", func() error { return b.Travel(wall, game.Position{}, nil) }, game.ErrWrongObjectType},
		{"missing travel payload", func() error { return b.Travel(&game.Object{Type: game.ObjectUnit}, game.Position{}, nil) }, game.ErrInvalidObjectData},
		{"invalid travel target", func() error { return b.Travel(unit, outside, nil) }, ErrInvalidPosition},
	}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if err := tc.call(); !errors.Is(err, tc.want) {
				t.Fatalf("got %v, want %v", err, tc.want)
			}
			if actions := b.connection.ActionQueue().Drain(); len(actions) != 0 {
				t.Fatalf("invalid action queued: %#v", actions)
			}
		})
	}
}

func TestActionsLeaveGameplayValidationToServer(t *testing.T) {
	b := connectedTestBot(t)
	// Foreign, cooling down, empty-handed, and far from the target: these
	// conditions must not prevent local queuing.
	unit := testUnit(7, 0, 0, 2, game.UnitProperties{})
	data, err := unit.GetUnitData()
	if err != nil {
		t.Fatal(err)
	}
	data.ActionCooldown = 10
	target := &game.Object{ID: 8, Type: game.ObjectCore, Pos: game.Position{X: 3, Y: 3}}
	b.game.Objects = []*game.Object{unit, target}
	for _, err := range []error{
		b.Move(unit, target.Pos),
		b.Attack(unit, target),
		b.TransferGems(unit, target.Pos, 100),
		b.TransferGems(target, unit.Pos, 100),
		b.CreateUnit("Invalid design", "unknown-component"),
		b.Travel(unit, game.Position{X: 1}, nil),
	} {
		if err != nil {
			t.Fatal(err)
		}
	}
	if actions := b.connection.ActionQueue().Drain(); len(actions) != 6 {
		t.Fatalf("expected six queued actions, got %#v", actions)
	}
}

func TestActionsRejectUnavailableClient(t *testing.T) {
	closed := connectedTestBot(t)
	if err := closed.Close(); err != nil {
		t.Fatal(err)
	}
	for _, b := range []*Bot{nil, {}, closed} {
		calls := []func() error{
			func() error { return b.CreateUnit("test") },
			func() error { return b.Move(nil, game.Position{}) },
			func() error { return b.Attack(nil, nil) },
			func() error { return b.TransferGems(nil, game.Position{}, 1) },
			func() error { return b.Travel(nil, game.Position{}, nil) },
		}
		for _, call := range calls {
			if err := call(); !errors.Is(err, ErrClientUnavailable) {
				t.Fatalf("got %v, want unavailable client", err)
			}
		}
	}
}
