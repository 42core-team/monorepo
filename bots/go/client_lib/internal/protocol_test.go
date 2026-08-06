package internal

import (
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func TestGameTickDecodesCurrentUnitModelAndDiffs(t *testing.T) {
	g := &game.Game{}
	initial, err := parseGameTick(`{"tick":10,"objects":[{"properties":{"damageObject":8,"maxGems":30,"gemsPerCooldownStep":5,"damageReductionPercent":25},"components":["combat","armor"],"name":"Ada","ActionCooldown":-2,"gems":4,"teamId":3,"hp":19,"y":2,"x":1,"type":1,"id":7}],"errors":[]}`)
	if err != nil {
		t.Fatal(err)
	}
	initial.applyTo(g)
	if g.ElapsedTicks != 10 || len(g.Objects) != 1 {
		t.Fatalf("unexpected game: %#v", g)
	}
	unit := g.Objects[0]
	data := unit.GetUnitData()
	if unit.Hp != 19 || data == nil || data.ActionCooldown != -2 || data.Name != "Ada" || data.Properties.DamageObject != 8 || len(data.Components) != 2 {
		t.Fatalf("unit not fully decoded: %#v / %#v", unit, data)
	}

	diff, err := parseGameTick(`{"tick":11,"objects":[{"id":7,"ActionCooldown":-3,"gems":6,"properties":{"maxGems":40}}],"errors":[]}`)
	if err != nil {
		t.Fatal(err)
	}
	diff.applyTo(g)
	data = unit.GetUnitData()
	if data.ActionCooldown != -3 || data.Gems != 6 {
		t.Fatalf("diff not applied: %#v", data)
	}
	if data.Name != "Ada" || data.Properties.DamageObject != 8 || data.Properties.MaxGems != 40 || data.Properties.GemsPerCooldownStep != 5 {
		t.Fatalf("omitted fields were lost: %#v", data)
	}

	dead, _ := parseGameTick(`{"tick":12,"objects":[{"id":7,"state":"dead"}],"errors":[]}`)
	dead.applyTo(g)
	if len(g.Objects) != 0 {
		t.Fatalf("dead object retained: %#v", g.Objects)
	}
}

func TestGameTickDoesNotLocallyDecrementCooldowns(t *testing.T) {
	g := &game.Game{Objects: []*game.Object{{ID: 1, Type: game.ObjectUnit, Hp: 10, ObjectData: &game.UnitData{ActionCooldown: 4}}}}
	tick, _ := parseGameTick(`{"tick":2,"objects":[],"errors":[]}`)
	tick.applyTo(g)
	if got := g.Objects[0].GetUnitData().ActionCooldown; got != 4 {
		t.Fatalf("cooldown changed without a server diff: %d", got)
	}
}
