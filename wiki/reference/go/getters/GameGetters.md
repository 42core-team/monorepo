---
title: "⚙️ Game getters"
permalink: "go_GameGetters"
sidebarTitle: "⚙️ Game getters"
---

```go
func (g *Game) ObjectByID(id uint) (*Object, error)
func (g *Game) ObjectAtPos(pos Position) *Object
func (g *Game) ObjectsFilter(predicate func(*Object) bool) []*Object
func (g *Game) ObjectsFilterCount(predicate func(*Object) bool) uint
func (g *Game) UnitsByName(name string) []*Object
func (g *Game) UnitsByNameCount(name string) uint
func (g *Game) NearestObject(from Position, predicate func(*Object) bool) *Object
func (g *Game) TeamUnits() []*Object
func (g *Game) EnemyUnits() []*Object
func (g *Game) MyCore() *Object
func (g *Game) EnemyCore() *Object
```

A nil predicate returns no results from `ObjectsFilter` and `NearestObject`; `ObjectsFilterCount(nil)` counts every object. `ObjectAtPos`, `NearestObject`, core getters, and name lookup return nil when no object matches. `ObjectByID` returns an error when absent. Name lookup includes only your team's units.

```go
deposits := g.ObjectsFilter(func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
nearest := g.NearestObject(unit.Pos, func(obj *game.Object) bool {
	return obj.Type == game.ObjectDeposit
})
```
