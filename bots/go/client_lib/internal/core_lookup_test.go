package internal

import (
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestOwnCoreLookup(t *testing.T) {
	own := &game.Object{Type: game.ObjectCore, Hp: 10, Pos: game.Position{X: 1}, ObjectData: &game.CoreData{TeamID: 7}}
	c := &Connection{game: &game.Game{MyTeamID: 7, Objects: []*game.Object{
		nil, {Type: game.ObjectCore},
		{Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 8}},
		{Type: game.ObjectCore, Pos: game.Position{X: 5}, ObjectData: &game.CoreData{TeamID: 7}},
		own,
	}}}
	if c.ownCore() != own {
		t.Fatal("internal lookup must select the nearest valid own core")
	}
	c.game.Objects = nil
	if c.ownCore() != nil {
		t.Fatal("missing own core should return nil")
	}
}
