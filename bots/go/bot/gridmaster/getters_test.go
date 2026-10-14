package main

import (
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestGetters(t *testing.T) {
	enemy := &game.Object{Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 8}}
	unit := &game.Object{Type: game.ObjectUnit, ObjectData: &game.UnitData{TeamID: 7}}
	g := &game.Game{MyTeamID: 7, Objects: []*game.Object{
		nil, {Type: game.ObjectCore}, {Type: game.ObjectUnit},
		{Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 0}},
		{Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 7}},
		{Type: game.ObjectUnit, ObjectData: &game.UnitData{TeamID: 8}},
		enemy, unit,
	}}
	if opponentCore(g) != enemy {
		t.Fatal("opponent core must exclude neutral, own, and invalid cores")
	}
	if units := ownUnits(g); len(units) != 1 || units[0] != unit {
		t.Fatalf("unexpected own units: %#v", units)
	}
	g.Objects = nil
	if opponentCore(g) != nil || ownUnits(g) != nil {
		t.Fatal("getters should return nil without matches")
	}
}
