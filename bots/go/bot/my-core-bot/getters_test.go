package main

import (
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestTeamGetters(t *testing.T) {
	own := &game.Object{ID: 1, Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 7}}
	enemy := &game.Object{ID: 2, Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 8}}
	ownUnit := &game.Object{ID: 3, Type: game.ObjectUnit, ObjectData: &game.UnitData{TeamID: 7}}
	enemyUnit := &game.Object{ID: 4, Type: game.ObjectUnit, ObjectData: &game.UnitData{TeamID: 8}}
	g := &game.Game{MyTeamID: 7, Objects: []*game.Object{
		nil,
		{Type: game.ObjectCore},
		{Type: game.ObjectUnit},
		{Type: game.ObjectCore, ObjectData: (*game.CoreData)(nil)},
		{Type: game.ObjectUnit, ObjectData: &game.CoreData{TeamID: 7}},
		{Type: game.ObjectCore, ObjectData: &game.CoreData{TeamID: 0}},
		{Type: game.ObjectUnit, ObjectData: &game.UnitData{TeamID: 0}},
		own, enemy, ownUnit, enemyUnit,
	}}
	if ownCore(g) != own || opponentCore(g) != enemy {
		t.Fatal("core getters must match valid team payloads")
	}
	if units := ownUnits(g); len(units) != 1 || units[0] != ownUnit {
		t.Fatalf("unexpected own units: %#v", units)
	}
	if units := opponentUnits(g); len(units) != 1 || units[0] != enemyUnit {
		t.Fatalf("unexpected opponent units: %#v", units)
	}
	if nearestOpponentUnit(g, game.Position{}) != enemyUnit {
		t.Fatal("nearest opponent must exclude neutral and invalid units")
	}
}

func TestGettersWithoutMatches(t *testing.T) {
	g := &game.Game{MyTeamID: 7, Objects: []*game.Object{nil}}
	for name, getter := range map[string]func(*game.Game) *game.Object{
		"own core": ownCore, "opponent core": opponentCore,
	} {
		if getter(g) != nil {
			t.Errorf("%s should return nil", name)
		}
	}
	if ownUnits(g) != nil || opponentUnits(g) != nil {
		t.Fatal("unmatched unit getters should return nil slices")
	}
	for name, getter := range map[string]func(*game.Game, game.Position) *game.Object{
		"deposit": nearestDeposit, "gem pile": nearestGemPile,
		"deposit or gem pile": nearestDepositOrGemPile, "opponent unit": nearestOpponentUnit,
	} {
		if getter(g, game.Position{}) != nil {
			t.Errorf("nearest %s should return nil", name)
		}
	}
}

func TestNearestGetters(t *testing.T) {
	deposit := &game.Object{Type: game.ObjectDeposit, Pos: game.Position{X: 3}}
	gems := &game.Object{Type: game.ObjectGemPile, Pos: game.Position{X: 1, Y: 1}}
	first := &game.Object{Type: game.ObjectUnit, Pos: game.Position{X: 2}, ObjectData: &game.UnitData{TeamID: 8}}
	tied := &game.Object{Type: game.ObjectUnit, Pos: game.Position{Y: 2}, ObjectData: &game.UnitData{TeamID: 9}}
	far := &game.Object{Type: game.ObjectUnit, Pos: game.Position{X: 4}, ObjectData: &game.UnitData{TeamID: 8}}
	core := &game.Object{Type: game.ObjectCore, Pos: game.Position{X: 1}, ObjectData: &game.CoreData{TeamID: 8}}
	farCore := &game.Object{Type: game.ObjectCore, Pos: game.Position{X: 5}, ObjectData: &game.CoreData{TeamID: 8}}
	g := &game.Game{MyTeamID: 7, Objects: []*game.Object{far, farCore, deposit, gems, first, tied, core}}
	if nearestDeposit(g, game.Position{}) != deposit || nearestGemPile(g, game.Position{}) != gems {
		t.Fatal("resource getters must filter by type")
	}
	if nearestDepositOrGemPile(g, game.Position{}) != gems {
		t.Fatal("combined resource getter must use Manhattan distance")
	}
	if nearestOpponentUnit(g, game.Position{}) != first {
		t.Fatal("nearest opponent must keep the first match on ties")
	}
	if opponentCore(g) != core {
		t.Fatal("core getter must select the nearest core to the origin")
	}
}
