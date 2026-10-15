package internal

import (
	"errors"
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestGameTickDecodesCurrentUnitModelAndDiffs(t *testing.T) {
	g := &game.Game{}
	initial, err := parseGameTick(`{"tick":10,"objects":[{"properties":{"damageObject":8,"maxGems":30,"gemsPerCooldownStep":5,"damageReductionPercent":25},"components":["combat","armor"],"name":"Ada","ActionCooldown":-2,"gems":4,"teamId":3,"hp":19,"y":2,"x":1,"type":1,"id":7}],"errors":[]}`)
	if err != nil {
		t.Fatal(err)
	}
	if err := initial.applyTo(g); err != nil {
		t.Fatal(err)
	}
	if g.ElapsedTicks != 10 || len(g.Objects) != 1 {
		t.Fatalf("unexpected game: %#v", g)
	}
	unit := g.Objects[0]
	data, err := unit.UnitData()
	if err != nil {
		t.Fatal(err)
	}
	if unit.Hp != 19 || data == nil || data.ActionCooldown != -2 || data.Name != "Ada" || data.Properties.DamageObject != 8 || len(data.Components) != 2 {
		t.Fatalf("unit not fully decoded: %#v / %#v", unit, data)
	}

	diff, err := parseGameTick(`{"tick":11,"objects":[{"id":7,"ActionCooldown":-3,"gems":6,"properties":{"maxGems":40}}],"errors":[]}`)
	if err != nil {
		t.Fatal(err)
	}
	if err := diff.applyTo(g); err != nil {
		t.Fatal(err)
	}
	data, err = unit.UnitData()
	if err != nil {
		t.Fatal(err)
	}
	if data.ActionCooldown != -3 || data.Gems != 6 {
		t.Fatalf("diff not applied: %#v", data)
	}
	if data.Name != "Ada" || data.Properties.DamageObject != 8 || data.Properties.MaxGems != 40 || data.Properties.GemsPerCooldownStep != 5 {
		t.Fatalf("omitted fields were lost: %#v", data)
	}

	dead, _ := parseGameTick(`{"tick":12,"objects":[{"id":7,"state":"dead"}],"errors":[]}`)
	if err := dead.applyTo(g); err != nil {
		t.Fatal(err)
	}
	if len(g.Objects) != 0 {
		t.Fatalf("dead object retained: %#v", g.Objects)
	}
}

func TestGameTickServerErrorsDoNotStopUpdates(t *testing.T) {
	g := &game.Game{}
	tick, err := parseGameTick(`{"tick":5,"objects":[],"errors":["unit is on action cooldown","insufficient gems"]}`)
	if err != nil {
		t.Fatal(err)
	}
	if err := tick.applyTo(g); err != nil {
		t.Fatal(err)
	}
	if g.ElapsedTicks != 5 {
		t.Fatalf("tick was not applied: %d", g.ElapsedTicks)
	}
	if err := (&gameTick{Tick: 6}).applyTo(g); err != nil {
		t.Fatal(err)
	}
	if g.ElapsedTicks != 6 {
		t.Fatalf("next tick was not applied: %d", g.ElapsedTicks)
	}
}

func TestGameTickRejectsMismatchedExistingPayload(t *testing.T) {
	for _, payload := range []game.ObjectData{nil, &game.CoreData{}, (*game.UnitData)(nil)} {
		g := &game.Game{Objects: []*game.Object{{ID: 1, Type: game.ObjectUnit, ObjectData: payload}}}
		id := uint(1)
		if err := (&gameTick{Tick: 2, Objects: []incomingObject{{ID: &id}}}).applyTo(g); !errors.Is(err, game.ErrInvalidObjectData) {
			t.Fatalf("got %v, want invalid object data", err)
		}
	}
}

func TestGameTickDoesNotLocallyDecrementCooldowns(t *testing.T) {
	g := &game.Game{Objects: []*game.Object{{ID: 1, Type: game.ObjectUnit, Hp: 10, ObjectData: &game.UnitData{ActionCooldown: 4}}}}
	tick, _ := parseGameTick(`{"tick":2,"objects":[],"errors":[]}`)
	if err := tick.applyTo(g); err != nil {
		t.Fatal(err)
	}
	data, err := g.Objects[0].UnitData()
	if err != nil {
		t.Fatal(err)
	}
	if got := data.ActionCooldown; got != 4 {
		t.Fatalf("cooldown changed without a server diff: %d", got)
	}
}

func TestGameTickKeepsPositionIndexCurrent(t *testing.T) {
	g := &game.Game{Config: game.Config{GridSize: 4}}
	for _, packet := range []string{
		`{"tick":1,"objects":[{"id":1,"type":3,"x":0,"y":0,"hp":5},{"id":2,"type":3,"x":2,"y":2,"hp":5}]}`,
		`{"tick":2,"objects":[{"id":1,"x":1,"y":0},{"id":2,"state":"dead"}]}`,
	} {
		tick, err := parseGameTick(packet)
		if err != nil {
			t.Fatal(err)
		}
		if err := tick.applyTo(g); err != nil {
			t.Fatal(err)
		}
	}
	if obj := g.ObjectAtPos(game.Position{X: 1, Y: 0}); obj == nil || obj.ID != 1 {
		t.Fatalf("moved object not indexed at new tile: %#v", obj)
	}
	if obj := g.ObjectAtPos(game.Position{}); obj != nil {
		t.Fatalf("moved object still indexed at old tile: %#v", obj)
	}
	if obj := g.ObjectAtPos(game.Position{X: 2, Y: 2}); obj != nil {
		t.Fatalf("dead object still indexed: %#v", obj)
	}
}
