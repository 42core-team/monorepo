package game

import (
	"fmt"
)

// A Game is the state of a running game as seen by one team.
//
// Methods that search Objects run in time linear in the number of objects,
// except [Game.ObjectAtPos], which uses a position index.
//
// Bots must not modify a Game or its objects, except for [Object.Data];
// the client updates them.
type Game struct {
	Config       Config    // configuration received when the game started
	Objects      []*Object // objects currently on the board
	ElapsedTicks uint      // number of ticks since the game started
	MyTeamID     uint      // team ID of this bot

	posIndex positionIndex // lookup table for ObjectAtPos, built on demand
}

// A positionIndex maps each tile, at y*GridSize+x, to the first object on it.
// It is valid while the state it was built from is unchanged.
type positionIndex struct {
	tiles    []*Object
	built    bool
	tick     uint
	gridSize uint
	count    int
	first    **Object // &Objects[0], detects a replaced Objects slice
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

// positionIndex returns the tiles of g's position index, rebuilding it if
// the state changed since it was built. Objects only change between ticks,
// so the index is rebuilt at most once per tick.
func (g *Game) positionIndex() []*Object {
	var first **Object
	if len(g.Objects) > 0 {
		first = &g.Objects[0]
	}
	index := &g.posIndex
	if index.built && index.tick == g.ElapsedTicks && index.gridSize == g.Config.GridSize &&
		index.count == len(g.Objects) && index.first == first {
		return index.tiles
	}
	size := int(g.Config.GridSize) * int(g.Config.GridSize)
	if cap(index.tiles) < size {
		index.tiles = make([]*Object, size)
	} else {
		index.tiles = index.tiles[:size]
		clear(index.tiles)
	}
	for _, obj := range g.Objects {
		if obj == nil || !g.IsPosValid(obj.Pos) {
			continue
		}
		tile := int(obj.Pos.Y)*int(g.Config.GridSize) + int(obj.Pos.X)
		if index.tiles[tile] == nil {
			index.tiles[tile] = obj
		}
	}
	index.built, index.tick, index.gridSize, index.count, index.first = true, g.ElapsedTicks, g.Config.GridSize, len(g.Objects), first
	return index.tiles
}

// ObjectAtPos returns the object at pos, or nil if the tile is empty or pos
// is not on the board. If several objects share a tile, it returns the one
// that appears first in g.Objects. Lookups take constant time, apart from
// the first lookup after each state update, which builds an index.
func (g *Game) ObjectAtPos(pos Position) *Object {
	if !g.IsPosValid(pos) {
		return nil
	}
	return g.positionIndex()[int(pos.Y)*int(g.Config.GridSize)+int(pos.X)]
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
// the minimum number of moves between them on an empty board.
func (g *Game) Distance(pos1, pos2 Position) int {
	dx := int(pos1.X) - int(pos2.X)
	dy := int(pos1.Y) - int(pos2.Y)
	if dx < 0 {
		dx = -dx
	}
	if dy < 0 {
		dy = -dy
	}
	return dx + dy
}
