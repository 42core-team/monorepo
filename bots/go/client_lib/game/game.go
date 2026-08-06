package game

import (
	"fmt"
)

type Game struct {
	Config       Config
	Objects      []*Object
	ElapsedTicks uint
	MyTeamID     uint
}

func (g *Game) ObjectByID(id uint) (*Object, error) {
	for _, obj := range g.Objects {
		if obj.ID == id {
			return obj, nil
		}
	}
	return nil, fmt.Errorf("object with id %d not found", id)
}

func (g *Game) ObjectAtPos(pos Position) *Object {
	for _, obj := range g.Objects {
		if obj.Pos.X == pos.X && obj.Pos.Y == pos.Y {
			return obj
		}
	}
	return nil
}

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

func (g *Game) ObjectsFilterCount(predicate func(*Object) bool) uint {
	var count uint
	for _, obj := range g.Objects {
		if predicate == nil || predicate(obj) {
			count++
		}
	}
	return count
}

func (g *Game) UnitsByName(name string) []*Object {
	if name == "" {
		return nil
	}
	return g.ObjectsFilter(func(obj *Object) bool {
		data := obj.GetUnitData()
		return data != nil && data.TeamID == g.MyTeamID && data.Name == name
	})
}

func (g *Game) UnitsByNameCount(name string) uint {
	if name == "" {
		return 0
	}
	return g.ObjectsFilterCount(func(obj *Object) bool {
		data := obj.GetUnitData()
		return data != nil && data.TeamID == g.MyTeamID && data.Name == name
	})
}

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

func (g *Game) TeamUnits() []*Object {
	return g.ObjectsFilter(func(obj *Object) bool {
		if obj.Type != ObjectUnit {
			return false
		}
		if data := obj.GetUnitData(); data != nil {
			return data.TeamID == g.MyTeamID
		}
		return false
	})
}

func (g *Game) EnemyUnits() []*Object {
	return g.ObjectsFilter(func(obj *Object) bool {
		if obj.Type != ObjectUnit {
			return false
		}
		if data := obj.GetUnitData(); data != nil {
			return data.TeamID != g.MyTeamID && data.TeamID != 0
		}
		return false
	})
}

func (g *Game) MyCore() *Object {
	return g.NearestObject(Position{X: 0, Y: 0}, func(obj *Object) bool {
		if obj.Type != ObjectCore {
			return false
		}
		if data := obj.GetCoreData(); data != nil {
			return data.TeamID == g.MyTeamID
		}
		return false
	})
}

func (g *Game) EnemyCore() *Object {
	return g.NearestObject(Position{X: 0, Y: 0}, func(obj *Object) bool {
		if obj.Type != ObjectCore {
			return false
		}
		if data := obj.GetCoreData(); data != nil {
			return data.TeamID != g.MyTeamID && data.TeamID != 0
		}
		return false
	})
}

func (g *Game) IsPosValid(pos Position) bool {
	return pos.X < g.Config.GridSize && pos.Y < g.Config.GridSize
}

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
