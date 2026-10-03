package game

import "testing"

func TestNameGettersReturnOnlyOwnedUnits(t *testing.T) {
	g := &Game{MyTeamID: 7, Objects: []*Object{
		{ID: 1, Type: ObjectUnit, ObjectData: &UnitData{TeamID: 7, Name: "Miner"}},
		{ID: 2, Type: ObjectUnit, ObjectData: &UnitData{TeamID: 8, Name: "Miner"}},
		{ID: 3, Type: ObjectUnit, ObjectData: &UnitData{TeamID: 7, Name: "Scout"}},
	}}
	miners := g.UnitsByName("Miner")
	if len(miners) != 1 || miners[0].ID != 1 {
		t.Fatalf("unexpected matching units: %#v", miners)
	}
	if count := g.UnitsByNameCount("Miner"); count != 1 {
		t.Fatalf("unexpected matching count: %d", count)
	}
	if g.UnitsByName("") != nil || g.UnitsByNameCount("") != 0 {
		t.Fatal("empty names should not match")
	}
}

func TestNilPredicateSemanticsMatchCLibrary(t *testing.T) {
	g := &Game{Objects: []*Object{{ID: 1}, {ID: 2}}}
	if objects := g.ObjectsFilter(nil); objects != nil {
		t.Fatalf("nil filter returned objects: %#v", objects)
	}
	if nearest := g.NearestObject(Position{}, nil); nearest != nil {
		t.Fatalf("nil nearest filter returned object: %#v", nearest)
	}
	if count := g.ObjectsFilterCount(nil); count != 2 {
		t.Fatalf("nil count filter returned %d", count)
	}
}
