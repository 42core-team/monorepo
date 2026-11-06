package game

import (
	"fmt"
	"math"
)

// A Game is the state of a running game as seen by one team.
//
// Methods that search Objects run in time linear in the number of objects.
type Game struct {
	Config       Config    // configuration received when the game started
	Objects      []*Object // objects currently on the board
	ElapsedTicks uint      // number of ticks since the game started
	MyTeamID     uint      // team ID of this bot
}

// ObjectByID returns the object with the given id.
// It returns an error if no such object exists.
func (g *Game) ObjectByID(id uint) (*Object, error) {
	for _, obj := range g.Objects {
		if obj.ID == id {
			return obj, nil
		}
	}
	return nil, fmt.Errorf("object with id %d not found", id)
}

// ObjectAtPos returns the object at pos, or nil if the tile is empty.
func (g *Game) ObjectAtPos(pos Position) *Object {
	for _, obj := range g.Objects {
		if obj.Pos.X == pos.X && obj.Pos.Y == pos.Y {
			return obj
		}
	}
	return nil
}

// ObjectsFilter returns all objects for which predicate returns true,
// in the order of g.Objects. It returns nil if predicate is nil.
func (g *Game) ObjectsFilter(predicate func(*Object) bool) []*Object {
	if predicate == nil {
		return nil
	}
	var result []*Object
	for _, obj := range g.Objects {
		if predicate(obj) {
			result = append(result, obj)
		}
	}
	return result
}

// ObjectsFilterCount returns the number of objects for which predicate
// returns true. A nil predicate matches every object.
func (g *Game) ObjectsFilterCount(predicate func(*Object) bool) uint {
	var count uint
	for _, obj := range g.Objects {
		if predicate == nil || predicate(obj) {
			count++
		}
	}
	return count
}

// UnitsByName returns the units of team g.MyTeamID whose name is name.
// It returns nil if name is empty.
func (g *Game) UnitsByName(name string) []*Object {
	if name == "" {
		return nil
	}
	return g.ObjectsFilter(func(obj *Object) bool {
		if obj == nil || obj.Type != ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		return err == nil && data.TeamID == g.MyTeamID && data.Name == name
	})
}

// UnitsByNameCount returns the number of units of team g.MyTeamID whose
// name is name. It returns 0 if name is empty.
func (g *Game) UnitsByNameCount(name string) uint {
	if name == "" {
		return 0
	}
	return g.ObjectsFilterCount(func(obj *Object) bool {
		if obj == nil || obj.Type != ObjectUnit {
			return false
		}
		data, err := obj.UnitData()
		return err == nil && data.TeamID == g.MyTeamID && data.Name == name
	})
}

// NearestObject returns the object closest to fromPos for which predicate
// returns true, measured by [Game.Distance]. Ties are broken in favor of the
// object that appears first in g.Objects. It returns nil if no object
// matches or predicate is nil.
func (g *Game) NearestObject(fromPos Position, predicate func(*Object) bool) *Object {
	objects := g.ObjectsFilter(predicate)
	if len(objects) == 0 {
		return nil
	}

	var nearest *Object
	minDistance := -1
	for _, obj := range objects {
		dist := g.Distance(fromPos, obj.Pos)
		if minDistance < 0 || dist < minDistance {
			minDistance = dist
			nearest = obj
		}
	}
	return nearest
}

// IsPosValid reports whether pos lies on the board.
func (g *Game) IsPosValid(pos Position) bool {
	return pos.X < g.Config.GridSize && pos.Y < g.Config.GridSize
}

// Distance returns the Manhattan distance between pos1 and pos2, which is
// the minimum number of moves between them on an empty board. A distance that
// does not fit in an int is reported as [math.MaxInt].
func (g *Game) Distance(pos1, pos2 Position) int {
	dx := absDiff(pos1.X, pos2.X)
	dy := absDiff(pos1.Y, pos2.Y)
	distance := dx + dy
	if distance < dx || distance > math.MaxInt {
		return math.MaxInt
	}
	return int(distance)
}

func absDiff(a, b uint) uint {
	if a > b {
		return a - b
	}
	return b - a
}
