package coregame

import (
	"testing"

	"github.com/42core-team/go-client-lib/game"
)

func testUnit(id, x, y, team uint, properties game.UnitProperties) *game.Object {
	return &game.Object{ID: id, Type: game.ObjectUnit, Pos: game.Position{X: x, Y: y}, Hp: 10, ObjectData: &game.UnitData{TeamID: team, Properties: properties}}
}

func TestTravelDefaultAttacksCheaperObstacleAndBuildsDebugPath(t *testing.T) {
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{DamageObject: 10})
	wall := &game.Object{ID: 2, Type: game.ObjectWall, Pos: game.Position{X: 1, Y: 0}, Hp: 10}
	g := &game.Game{Config: game.Config{GridSize: 3}, Objects: []*game.Object{unit, wall}}
	plan, ok := makeTravelPlan(g, unit, game.Position{X: 2, Y: 0}, nil)
	if !ok || len(plan.path) != 2 || plan.path[0] != wall.Pos || plan.obstacle != wall || plan.action != TravelAttack {
		t.Fatalf("unexpected plan: %#v, ok=%v", plan, ok)
	}
}

func TestTravelDefaultWaitsForFriendlyUnit(t *testing.T) {
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{DamageUnit: 10})
	friendly := testUnit(2, 1, 0, 1, game.UnitProperties{})
	g := &game.Game{Config: game.Config{GridSize: 3}, Objects: []*game.Object{unit, friendly}}
	plan, ok := makeTravelPlan(g, unit, game.Position{X: 2, Y: 0}, nil)
	if !ok || plan.path[0] != friendly.Pos || plan.obstacle != friendly || plan.action != TravelPass {
		t.Fatalf("unexpected plan: %#v, ok=%v", plan, ok)
	}
}

func TestTravelCustomPolicySeparatesPassAttackAndBlock(t *testing.T) {
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{})
	friendly := testUnit(2, 1, 0, 1, game.UnitProperties{})
	g := &game.Game{Config: game.Config{GridSize: 2}, Objects: []*game.Object{unit, friendly}}
	plan, ok := makeTravelPlan(g, unit, friendly.Pos, func(game.Position, *game.Object) TileTravelability {
		return TileTravelability{Weight: 1, Action: TravelAttack}
	})
	if !ok || plan.obstacle != friendly || plan.action != TravelAttack {
		t.Fatalf("unexpected attack plan: %#v, ok=%v", plan, ok)
	}
	_, ok = makeTravelPlan(g, unit, friendly.Pos, func(pos game.Position, _ *game.Object) TileTravelability {
		if pos == friendly.Pos {
			return TileTravelability{Action: TravelBlock}
		}
		return TileTravelability{Weight: 1, Action: TravelPass}
	})
	if ok {
		t.Fatal("blocked destination unexpectedly produced a plan")
	}
}

func TestTravelPlansDuringPositiveCooldown(t *testing.T) {
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{})
	unit.GetUnitData().ActionCooldown = 1
	g := &game.Game{Config: game.Config{GridSize: 2}, Objects: []*game.Object{unit}}
	if _, ok := makeTravelPlan(g, unit, game.Position{X: 1, Y: 0}, nil); !ok {
		t.Fatal("travel should leave cooldown checks to caller/server")
	}
}

func TestDefaultTravelabilityBlocksFullGemCarrier(t *testing.T) {
	unit := testUnit(1, 0, 0, 1, game.UnitProperties{MaxGems: 5})
	unit.GetUnitData().Gems = 5
	pile := &game.Object{ID: 2, Type: game.ObjectGemPile, Pos: game.Position{X: 1, Y: 0}, Hp: 1, ObjectData: &game.DepositData{Gems: 10}}
	g := &game.Game{Config: game.Config{GridSize: 2}, Objects: []*game.Object{unit, pile}}
	if got := defaultTileTravelability(g, pile.Pos, unit); got.Action != TravelBlock {
		t.Fatalf("got travelability %#v", got)
	}
}
